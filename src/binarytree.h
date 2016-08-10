#ifndef BINARY_TREE_
#define BINARY_TREE_

#include <memory>
#include <functional>
#include <iterator>
#include <list>

/*!\brief Forward declaration of BinaryTreeIterator */
template <typename T>
class BinaryTreeIterator;


/*! \brief Binary tree holding values T.
 *
 * nullptr terminates the tree.
 */
template <typename T>
class BinaryTree
{
    public:
        friend class BinaryTreeIterator<T>;

        /*!\brief Constructing an Binary Tree with a single value sets branches null.
         * \param[in] value The value for this node.
         */
        BinaryTree(T value) :
            left_ {nullptr}, right_ {nullptr}, value_ {value}
        {};
        /*!\brief Construct Binary Tree with left and right branches and a value for this root.
         * \param[in] left Unqiue_ptr to the left branch.
         * \param[in] right Unique_ptr to the right brnach.
         * \param[in] value The value for this node.
         */
        BinaryTree(std::unique_ptr < BinaryTree < T>> left, std::unique_ptr < BinaryTree < T>> right, T value) :
            left_ {std::move(left)}, right_ {std::move(right)}, value_ {value}
        {};
        /*!\brief Move constructor for binary tree.
         * \param[in] other The other tree.
         */
        BinaryTree(BinaryTree &&other) :
            left_ {std::move(other.left_)}, right_ {std::move(other.right_)}, value_ {std::move(other.value_)}
        {};

        /*!\brief Access operator
         * \returns a reference to the value this node holds.
         */
        T &operator* () {return value_; };

        /*!\brief Beginning from the top, removes complete branches for which cut_criterium(value) is true.
         * If a branch is removed, it is not further checked.
         * Nodes further down a cut branch may not fulfill cut_criterium.
         *
         * \returns list of unique_ptrs to the cut branches.
         */
        template <typename UnaryPredicate>
        std::list < std::unique_ptr < BinaryTree<T>>> cut(UnaryPredicate cut_criterium)
        {
            std::list < std::unique_ptr < BinaryTree < T>>> result;
            for(auto & branch : *this)
            {
                if (branch.left_ != nullptr && cut_criterium(**(branch.left_)) == true)
                {
                    result.push_back(std::move(branch.left_));
                }
                if (branch.right_ != nullptr && cut_criterium(**(branch.right_)) == true)
                {
                    result.push_back(std::move(branch.right_));
                }
            }
            return result;
        };


        /*!\brief Beginning from the top, removes complete branches for which cut_criterium(value) is true.
         * If a branch is removed, it is not further checked.
         * Nodes further down a cut branch may not fulfill cut_criterium.
         *
         * \returns list of unique_ptrs to the cut branches.
         */
        template <typename ComparisonFunction>
        BinaryTreeIterator<T> max_element(ComparisonFunction compare)
        {
            BinaryTreeIterator<T> result{this};
            for(auto & branch : *this)
            {
                if (compare(branch.value_,*result) == true )
                {
                    result(*branch);
                }
            }
            return result;
        };
        /*!\brief The bottom of the tree, i.e. all nodes for which left and right are null.
         * \returns a list of observing pointers to the elements at the bottom of the binary tree.
         */
        std::list <  BinaryTree<T>*> bottom()
        {
            std::list <  BinaryTree<T>*> result;
            for (auto &branch : *this)
            {
                if ((branch.left_ == nullptr) && (branch.right_ == nullptr))
                {
                    result.push_back(&branch);
                }
            }
            return result;
        }
        /*!\brief Iterator to the begin of the binary tree. */
        BinaryTreeIterator<T> begin()
        {
            return BinaryTreeIterator<T>(this);
        };

        /*!\brief Iterator to the begin of the binary tree. */
        BinaryTreeIterator<T> end()
        {
            return BinaryTreeIterator<T>(nullptr);
        };

    private:
        std::unique_ptr < BinaryTree < T>> left_;//< This tree owns its left branch.
        std::unique_ptr < BinaryTree < T>> right_;//< and its right branch.
        T value_; //< The value to be stored.
};


/*!\brief Iterator class for binary trees.
 *
 * This iterator accesses all tree nodes, following the leftmost part from the top,
 * storing junctions to the right along the way. When at the bottom, it goes to the
 * next right node in the list and follows the leftmost path from there.
 */
template <typename T>
class BinaryTreeIterator : public std::iterator<std::forward_iterator_tag, T>
{
    public:
        /*!\brief Construct from Binary Tree.
         * \param[in] Observing pointer to tree to step through.
         */
        BinaryTreeIterator<T> (BinaryTree<T> * tree) : current_branch_ {tree}
        {};
        /*!\brief Step through Binary Tree. */
        BinaryTreeIterator<T> &operator++()
        {
            if (current_branch_->left_ == nullptr)
            {
                if (current_branch_->right_ == nullptr)
                {
                    // at a bottom of a branch, jump to the next branch if exists
                    if (!skipped_branches.empty())
                    {
                        current_branch_ = skipped_branches.front();
                        skipped_branches.pop_front();
                    }
                    else
                    {
                        // at the end of the tree
                        current_branch_ = nullptr;
                    }
                }
                else
                {
                    current_branch_ = current_branch_->right_.get();
                }
            }
            else
            {
                if (current_branch_->right_ != nullptr)
                {
                    skipped_branches.push_back(current_branch_->right_.get());
                }
                current_branch_ = current_branch_->left_.get();
            }
            return *this;
        }
        /*!\brief Compare two iterators
         * \param[in] rhs Other iterator to compare to.
         */
        bool operator== (const BinaryTreeIterator &rhs)
        {
            return (current_branch_ == rhs.current_branch_);
        }
        /*!\brief Compare two iterators unequal.
         * \param[in] rhs Other iterator to compare to.
         * Implemented as !(==)
         */
        bool operator!= (const BinaryTreeIterator &rhs)
        {
            return !(*this == rhs);
        }
        /*!\brief Access element.
         * \returns Reference to Binary Tree.
         */
        BinaryTree<T> &operator*()
        {
            return *current_branch_;
        }
    private:
        BinaryTree<T>           * current_branch_; //< current iterator position in tree
        std::list<BinaryTree<T>*> skipped_branches;   //< list of "right" branches to be visitied later.
};

#endif /* end of include guard: BINARY_TREE_ */
