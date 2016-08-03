/*
 * cluster.c
 *
 *  Created on: Apr 29, 2014
 *      Author: cblau
 */

#include "stdlib.h"
#include "float.h"
#include "math.h"
#include "stdint.h"
#include "stdio.h"
#include "cluster.h"
#include <list>

int pos_unsorted(int * seq, int n)
{
    int result;
    for (result = 0; result < n - 1; ++result)
    {
        if (seq[result] > seq[result + 1])
        {
            return result;
        }
    }
    return result;
}

#define SWAP(type, x, y) {type _tmp = x; x = y; y = _tmp; }

/* sort matrix entries according to given sequence*
 * i.e. for seq=[1,3,2], second and third row and column are swapped */
int matrix_resort(real ** m, int * seq, int n)
{
    /* bubble sort */
    int i;
    for (i = pos_unsorted(seq, n); i < n - 1; i = pos_unsorted(seq, n))
    {
        SWAP(real *, m[i], m[i + 1]);
        // swap columns
        int i_row;
        for (i_row = 0; i_row < n; ++i_row)
        {
            SWAP(real, m[i_row][i], m[i_row][i + 1]);
        }
        SWAP(int, seq[i], seq[i + 1]);
    }
    return EXIT_SUCCESS;
}

int print_row_int(FILE * stream, int n, int * m)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        fprintf(stream, "%d\t", m[i]);
    }
    fprintf(stream, "\n");
    return EXIT_SUCCESS;
}

int print_row(FILE * stream, int n, real * m)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        fprintf(stream, "%5.2f\t", m[i]);
    }
    fseek(stream, -1, SEEK_CUR);
    fprintf(stream, "\n");
    return EXIT_SUCCESS;
}

int print_matrix(FILE * stream, int n, real **m)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        print_row(stream, n, m[i]);
    }
    return EXIT_SUCCESS;
}

int remove_row(int n, real ** m, int row)
{
    int    i;
    real * tmp = m[row];
    for (i = row; i < n - 1; ++i)
    {
        m[i] = m[i + 1];
    }
    free(tmp);
    return EXIT_SUCCESS;
}

int remove_column(int n, real ** m, int column)
{
    int i;
    int j;
    for (i = 0; i < n - 1; ++i)
    {
        for (j = column; j < n - 1; ++j)
        {
            m[i][j] = m[i][j + 1];
        }
    }
    return EXIT_SUCCESS;
}

template <typename T>
T cluster_hierarchically(std::list<T> branches)
{
    while (branches.size() > 1)
    {
        auto left = std::min_element(cbegin(branches), cend(branches),
                         [] (const T &a, const T &b ){
                             return nextRightNeighbourDistance(a) < nextRightNeighbourDistance(b);
                         });
        auto right = left + nextRightNeighbour(left);
        for_each(begin(branches),right,[](T branch){updateDistances(branch,left,right);});
        *left = merge(left, right);
        branches.erase(right);
    }
    return branches.first();
}

int merge_clusters(int n, t_cluster ** clusters, real ai, real aj, real b,
                   real g)
{
    /* put the closest clusters first in clusters */
    if (n <= 1)
    {
        return EXIT_SUCCESS;
    }
    int     i, j;
    int     merge_i = 0, merge_j = 0;
    real ** d       = (*clusters)->d;
    real    mindist = FLT_MAX;
    for (i = 0; i < n; ++i)
    {
        for (j = i + 1; j < n; ++j)
        {
            if (d[i][j] < mindist)
            {
                mindist = d[i][j];
                merge_i = i;
                merge_j = j;
            }
        }
    }

    t_cluster * clust_i = clusters[merge_i];
    t_cluster * clust_j = clusters[merge_j];
    clust_i->nb = clust_j;
    clust_j->nb = clust_i;

    t_cluster * merged = malloc(sizeof(t_cluster));
    clust_i->parent = merged;
    clust_j->parent = merged;

    merged->merge_d  = mindist;
    merged->child[0] = clust_i;

    merged->child[1] = clust_j;

    merged->n = clust_i->n + clust_j->n;
    merged->d = clust_i->d;

    merged->rep = NULL;
    merged->id  = malloc(merged->n * sizeof(int));
    for (i = 0; i < clust_i->n; ++i)
    {
        merged->id[i] = clust_i->id[i];
    }
    for (i = 0; i < clust_j->n; ++i)
    {
        merged->id[i + clust_i->n] = clust_j->id[i];
    }

    merged->parent = NULL;

    real d_new[n];

    if ((ai < -0.99) && (ai > -1.1))
    {
        ai = (float) clust_i->n / (float) merged->n; //< centroid or group average
    }
    ;
    if ((aj < -0.99) && (aj > -1.1))
    {
        aj = (float) clust_j->n / (float) merged->n; //< centroid or group average
    }
    ;

    if ((b < -0.99) && (b > -1.11))   //< centroid
    {
        b = -ai * aj;
    }

    if (ai > -1.5)
    {
        for (i = 0; i < n; ++i)
        {
            d_new[i] = ai * d[merge_i][i] + aj * d[merge_j][i]
                + b * d[merge_i][merge_j]
                + g * fabs(d[merge_i][i] - d[merge_j][i]);

        }
    }
    else     //< ward's minimum variance
    {
        for (i = 0; i < n; ++i)
        {
            ai = (clust_i->n + clusters[i]->n)
                / (float) (merged->n + clusters[i]->n);
            aj = (clust_j->n + clusters[i]->n)
                / (float) (merged->n + clusters[i]->n);
            b        = -clusters[i]->n / (float) (merged->n + clusters[i]->n);
            d_new[i] = ai * d[merge_i][i] + aj * d[merge_j][i]
                + b * d[merge_i][merge_j]
                + g * fabs(d[merge_i][i] - d[merge_j][i]);

        }
    }

    clusters[merge_i] = merged;

    for (i = 0; i < n; ++i)
    {
        d[i][merge_i] = d_new[i];
        d[merge_i][i] = d_new[i];
    }
    for (i = merge_j; i < n - 1; ++i)
    {
        clusters[i] = clusters[i + 1];
    }

    remove_row(n, d, merge_j);

    remove_column(n, d, merge_j);

    return EXIT_SUCCESS;
}

// apply f to each element in tree.
// stop if f fails, return element at which f failes, NULL if no failure
t_cluster * traverse_tree(void * thing, t_cluster * root,
                          int (*f)(void * thing, t_cluster * current_leaf, t_cluster * root))
{
    cluster_list_t * t = malloc(sizeof(cluster_list_t));
    cluster_list_t * i_t;
    t_cluster      * result = NULL;
    t->this = root;
    t->next = NULL;

    while (t != NULL)
    {
        if (f(thing, t->this, root) == EXIT_FAILURE)
        {
            result = t->this;
            while (t != NULL)
            {
                i_t = t;
                t   = t->next;
                free(i_t);
            }

            return result;
        }
        if ((t->this->child)[0] != NULL)
        {
            // put to the front;
            i_t           = t->next;
            t->next       = malloc(sizeof(cluster_list_t));
            t->next->this = (t->this->child)[0];
            t->next->next = i_t;
        }
        if ((t->this->child)[1] != NULL)
        {
            for (i_t = t; i_t->next != NULL; i_t = i_t->next)
            {
                ;
            }
            i_t->next       = malloc(sizeof(cluster_list_t));
            i_t->next->this = (t->this->child)[1];
            i_t->next->next = NULL;
        }
        i_t = t;
        t   = t->next;
        free(i_t);
    }
    return result;
}

int findmax_d(void * thing, t_cluster * x, t_cluster * root)
{
    real * max_merge_d = (float *) thing;
    *max_merge_d = max(x->merge_d, *max_merge_d);
    return EXIT_SUCCESS;
}

int calc_cluster_reps(void * thing, t_cluster * root)
{
    traverse_tree(thing, root, &avg_per_cluster);
    return EXIT_SUCCESS;
}

int insert_if_bottom(void * thing, t_cluster * x, t_cluster * root)
{
    cluster_list_t ** list = (cluster_list_t **) thing;
    if ((x->child[0] == NULL) && (x->child[1] == NULL))
    {
        cluster_list_t * tmp = malloc(sizeof(cluster_list_t));
        tmp->this = x;
        tmp->next = *list;
        *list     = tmp;
    }
    return EXIT_SUCCESS;
}

cluster_list_t * get_bottom(t_cluster * root)
{
    cluster_list_t * result = NULL;
    traverse_tree(&result, root, &insert_if_bottom);
    return result;
}

int merge_d_over_threshold(void * thing, t_cluster * x, t_cluster * root)
{
    real * threshold = (real*) thing;
    if (x->parent == NULL)
    {
        return EXIT_SUCCESS;
    }
    if (x->parent->merge_d > *threshold)
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

int cut_clusters(t_cluster * root, real threshold)
{
    t_cluster * x = traverse_tree(&threshold, root, &merge_d_over_threshold);
    t_cluster * parent;
    while (x != NULL)
    {
        parent = x->parent;
        if (parent == NULL)
        {
            return EXIT_SUCCESS; //< can't cut the root
        }
        free_tree(parent->child[0]);
        free_tree(parent->child[1]);
        parent->child[0] = NULL;
        parent->child[1] = NULL;
        x                = traverse_tree(&threshold, root, &merge_d_over_threshold);
    }
    return EXIT_SUCCESS;
}

int add_bottom_clusters(int * n, t_cluster *** all, t_cluster ** new,
                        int offset)
{
    cluster_list_t * bottom = get_bottom(*new);

    cluster_list_t * tmp;
    int              i;
    while (bottom != NULL)
    {
        *n            += 1;
        *all           = realloc(*all, *n * sizeof(t_cluster *));
        (*all)[*n - 1] = bottom->this;
        for (i = 0; i < bottom->this->n; ++i)
        {
            bottom->this->id[i] += offset;
        }
        if (bottom->this->parent != NULL)
        {
            if (bottom->this->parent->child[0] == bottom->this)
            {
                bottom->this->parent->child[0] = NULL;
            }
            else
            {
                bottom->this->parent->child[1] = NULL;
            }
        }
        else
        {
            *new = NULL; // if root equals bottom
        }
        tmp    = bottom;
        bottom = bottom->next;
        free(tmp);
    }
    return EXIT_SUCCESS;
}

int reset_merge_d(int n, t_cluster ** clusters)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        clusters[i]->merge_d = 0;
    }
    return EXIT_SUCCESS;
}


int free_tree(t_cluster * tree)
{

    cluster_list_t * bottom = get_bottom(tree);
    cluster_list_t * tmp;

    while (bottom->this != tree)
    {

        while (bottom != NULL)
        {
            if (bottom->this->parent->child[0] == bottom->this)
            {
                bottom->this->parent->child[0] = NULL;
            }
            else
            {
                bottom->this->parent->child[1] = NULL;
            }

            free_rep(bottom->this->rep);
            free(bottom->this->id);
            free(bottom->this);
            tmp    = bottom;
            bottom = bottom->next;
            free(tmp);
        }
        bottom = get_bottom(tree);
    }
    free_rep(tree->rep);
    free(tree->id);
    free(tree);
    free(bottom);

    return EXIT_SUCCESS;
}

t_cluster ** distance_matrix_to_clusters(int n, real ** m)
{
    int          i;
    t_cluster ** result = malloc(n * sizeof(t_cluster*));
    for (i = 0; (i < n); i++)
    {
        result[i]             = malloc(sizeof(t_cluster));
        result[i]->rep        = NULL;
        (result[i]->child)[0] = NULL;
        (result[i]->child)[1] = NULL;

        result[i]->merge_d = 0;
        result[i]->id      = malloc(sizeof(int));
        result[i]->id[0]   = i;
        result[i]->n       = 1;
        (result[i])->d     = m;
    }
    return result;
}

int hierarchical_clustering(t_cluster ** clusters, int n,
                            enum clust_method method)
{

    real ai, aj, b, g;
    switch (method)
    {
        case s_link:
            ai = 0.5;
            aj = 0.5;
            b  = 0;
            g  = -0.5;
            break;
        case c_link:
            ai = 0.5;
            aj = 0.5;
            b  = 0;
            g  = 0.5;
            break;
        case s_avg:
            ai = 0.5;
            aj = 0.5;
            b  = 0;
            g  = 0;
            break;
        case centroid:
            ai = -1;
            aj = -1;
            b  = -1;
            g  = 0;
            break;
        case median:
            ai = 0.5;
            aj = 0.5;
            b  = -0.25;
            g  = 0;
            break;
        case gr_avg:
            ai = -1;
            aj = -1;
            b  = 0;
            g  = 0;
            break;
        case ward:
            ai = -2;
            aj = -2;
            b  = -2;
            g  = 0;
            break;
        default:
            fprintf(stderr, "\nUnknown clustering method.\n ");
            return EXIT_FAILURE;
    }

    /* Merge closest clusters*/
    for (; n > 1; --n)
    {
        merge_clusters(n, clusters, ai, aj, b, g);
    }
    (*clusters)->nb     = NULL;
    (*clusters)->parent = NULL;
    free((*clusters)->d[0]);
    free((*clusters)->d);
    return EXIT_SUCCESS;
}

int cmpfunc(const void * a, const void * b)
{
    return (*(int*) a - *(int*) b);
}

int cluster_to_json(void * thing, t_cluster * x, t_cluster * root)
{
    FILE * outfile = (FILE*) thing;
    fprintf(outfile,
            "{\n\t\"id\":\"%p\",\n\t\"parent\":\"%p\",\n\t\"merge_d\":%g,"
            "\n", x, x->parent, x->merge_d);
    fprintf(outfile, "\t\"representative\":\"%p\",\n", x->rep);
    fprintf(outfile, "\t\"neighbour\":\"%p\",\n", x->nb);
    fprintf(outfile, "\t\"n_members\":%d,\n\t", x->n);
    int i;
    fprintf(outfile, "\"member_id\":[");
//    qsort(x->id, x->n, sizeof(int), &cmpfunc);
    for (i = 0; i < x->n - 1; ++i)
    {
        fprintf(outfile, "%d,", x->id[i]);
    }
    fprintf(outfile, "%d],\n\t", x->id[x->n - 1]);
    fprintf(outfile, "\"child_id\":[");
    for (i = 0; i < 1; ++i)
    {
        fprintf(outfile, "\"%p\",", x->child[i]);
    }
    fprintf(outfile, "\"%p\"]\n", x->child[1]);

    fprintf(outfile, "},\n");
    return EXIT_SUCCESS;
}


int representatives_to_json(t_cluster *tree, const char * jsonfile)
{
    if (jsonfile == NULL)
    {
        return EXIT_FAILURE;
    }
    FILE * outfile = fopen(jsonfile, "w");
    fprintf(outfile, "{\n\t");
    pairs_to_json(tree, outfile);
    fprintf(outfile, "\"matrices\":[\n");
    traverse_tree(outfile, tree, &clusterrep_to_json);
    fseek(outfile, -2, SEEK_CUR); // workaround for last comma
    fprintf(outfile, "\n\t]\n}");
    fclose(outfile);
    return EXIT_SUCCESS;
}

int dendrogram_to_json(t_cluster * tree, const char * jsonfile)
{
    if (jsonfile == NULL)
    {
        return EXIT_FAILURE;
    }
    FILE * outfile = fopen(jsonfile, "w");
    fprintf(outfile, "[");
    traverse_tree(outfile, tree, &cluster_to_json);
    fseek(outfile, -2, SEEK_CUR); // workaround for last comma
    fprintf(outfile, "]");
    fclose(outfile);
    return EXIT_SUCCESS;
}


int bottom_to_ndx(t_cluster *tree, const char * outfile)
{
    if (outfile == NULL)
    {
        return EXIT_FAILURE;
    }
    t_blocka       * b;
    b = new_blocka();
    char          ** gnames = NULL;

    cluster_list_t * tmp;
    cluster_list_t * bottom = get_bottom(tree);

    if (bottom == NULL)
    {
        return EXIT_FAILURE;
    }

    tmp = bottom; // remember the beginning
    char name[128];
    while (bottom != NULL)
    {
        sprintf(name, "%p", bottom->this);
        add_grp(b, &gnames, bottom->this->n, bottom->this->id, name);
        tmp    = bottom;
        bottom = bottom->next;
        free(tmp);
    }

    write_index(outfile, b, gnames);

    return EXIT_SUCCESS;
}

int count_list(void * list)
{
    int i = 0;
    while (((t_list*) list) != NULL)
    {
        list = ((t_list*) list)->next;
        ++i;
    }
    return i;
}

int min_merge_d(void * thing, t_cluster * x, t_cluster * root)
{
    if (x->parent == NULL)
    {
        return EXIT_SUCCESS;
    }
    if (x->parent->merge_d < *((float*) thing))
    {
        *((float*) thing) = x->parent->merge_d;
    }
    return EXIT_SUCCESS;
}

float smallest_merge_d(t_cluster * all_clusters)
{
    float result = FLT_MAX;
    traverse_tree(&result, all_clusters, &min_merge_d);
    return result;
}

int cut_clusters_n_at_bottom(t_cluster * all_clusters, int n)
{

    while (count_list(get_bottom(all_clusters)) > n)
    {
        cut_clusters(all_clusters,
                     smallest_merge_d(all_clusters) + 10 * FLT_MIN);
    }

    return EXIT_SUCCESS;
}

int bottom_representatives_to_json(t_cluster ** all_clusters,
                                   const char * outfilename)
{
    if (outfilename == NULL)
    {
        return EXIT_FAILURE;
    }
    int          i;
    int          n_bottom = 0;
    t_cluster ** bottom   = NULL;

    FILE       * outfilerep = fopen(outfilename, "w");
    add_bottom_clusters(&n_bottom, &bottom, all_clusters, 0);
    for (i = 0; i < n_bottom; ++i)
    {
        clusterrep_to_json(outfilerep, bottom[i], bottom[0]);
    }
    free(bottom);
    fclose(outfilerep);
    return EXIT_SUCCESS;
}
