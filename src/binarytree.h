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
                iterator (BinaryTree<T> * temp) : current_branch_(temp) {};
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
                T &operator*()
                {
                    return *(current_branch_->value_);
                }
            private:
                BinaryTree<T>           * current_branch_;
                std::list<BinaryTree<T>*> next_branches;
        };
        BinaryTree(std::unique_ptr<T> value) :
            left_(nullptr), right_(nullptr), value_(std::move(value)){};
        BinaryTree(std::unique_ptr<BinaryTree<T>> left,std::unique_ptr<BinaryTree<T>> right, std::unique_ptr<T> value) :
            left_(std::move(left)), right_(std::move(right)), value_(std::move(value))
        {
        };
        BinaryTree<T> left(){return *left_; };
        BinaryTree<T> right(){return *right_; };
        T &operator* () {return *value_; };
        iterator begin()
        {
            return iterator(this);
        };
        iterator end()
        {
            return iterator(nullptr);
        };

    private:
        std::unique_ptr<BinaryTree<T>> left_;
        std::unique_ptr<BinaryTree<T>> right_;
        std::unique_ptr<T>          value_;
};


#endif /* end of include guard: BINARY_TREE_ */
