#ifndef BINARY_TREE_
#define BINARY_TREE_

#include <memory>
#include <functional>
#include <iterator>
#include <list>



/*! \brief Binary tree of value type T.
 *
 * nullptr terminates the tree.
 */
template <typename T>
class BinaryTree
{
    public:
        class iterator : public std::iterator<std::forward_iterator_tag, T>
        {
            public:
                iterator (BinaryTree<T> temp) : current_branch_(&temp) {};
                iterator (const iterator &other) : current_branch_(other.current_branch_), next_branches(std::move(other.next_branches)) {};
                iterator &operator++()
                {
                    if (current_branch_->left_ == nullptr)
                    {
                        if (current_branch_->right_ == nullptr)
                        {
                            // at a bottom of a branch, jump to the next branch if exists
                            if (!next_branches.empty())
                            {
                                current_branch_ = next_branches.front();
                                next_branches.pop_front();
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
                            next_branches.push_back(current_branch_->right_.get());
                        }
                        current_branch_ = current_branch_->left_.get();
                    }
                    return *this;
                }
                bool operator== (const iterator &rhs)
                {
                    return (current_branch_ == rhs.current_branch_);
                }
                bool operator!= (const iterator &rhs)
                {
                    return !(*this == rhs);
                }
                T & operator*()
                {
                    return *(current_branch_->value);
                }
            private:
                BinaryTree<T>* current_branch_;
                std::list<BinaryTree<T>*> next_branches;
        };
        BinaryTree(std::unique_ptr<BinaryTree<T>> &&left, std::unique_ptr<BinaryTree<T>> &&right, std::unique_ptr<T> &&value) :
            left_(left), right_(right), value_(value){};
        BinaryTree(std::unique_ptr<T> &&value) :
            left_(nullptr), right_(nullptr), value_(value){};
        BinaryTree left(){return *left_; };
        BinaryTree right(){return *right_; };
        T & value() const {return *value_; } ;
        BinaryTree::iterator begin();
        BinaryTree::iterator end();

    private:
        std::unique_ptr<BinaryTree> right_;
        std::unique_ptr<BinaryTree> left_;
        std::unique_ptr<T>          value_;
};


#endif /* end of include guard: BINARY_TREE_ */
