#include "persistent_set.h"
#include <cassert>
#include <limits>


using namespace std;

persistent_set::node::node(value_type key) : key(key), left(nullptr), right(nullptr) {}
persistent_set::node::node(std::shared_ptr<node> const &other) : key(other->key), left(other->left), right(other->right) {}

persistent_set::node& persistent_set::node::operator=(const persistent_set::node& rhs) {
    key = rhs.key;
    left = rhs.left;
    right = rhs.right;
	return *this;
}

std::shared_ptr<persistent_set::node> persistent_set::node::insert(value_type x)
{
    if (key == x)
        return shared_from_this();
    else if (x < key) {
        if (left) {
            return (left = std::make_shared<node>(left))->insert(x);
        } else {
            return (left = std::make_shared<node>(x));
        }
    } else {
        if (right) {
            return (right = std::make_shared<node>(right))->insert(x);
        } else {
            return (right = std::make_shared<node>(x));
        }
    }
}

std::shared_ptr<persistent_set::node> persistent_set::node::find(persistent_set::value_type x)
{
    if (x == key) {
        return shared_from_this();
    } else if (x < key) {
        return left ? left->find(x) : nullptr;
    } else {
        return right ? right->find(x) : nullptr;
    }
}

void persistent_set::node::erase(std::shared_ptr<persistent_set::node> x, std::shared_ptr<persistent_set::node> par)
{
    if (key == x->key) {
        std::shared_ptr<node> y;
        if (left) {
            left = std::make_shared<node>(left);
            y = left;
            while (y->right) {
                y->right = std::make_shared<node>(y->right);
                y = y->right;
            }
            y->right = right;
            right = nullptr;
        }
        (par->left == shared_from_this() ? par->left : par->right) = (left ? left : right);
    } else if (x->key < key) {
        (left = std::make_shared<node>(left))->erase(x, shared_from_this());
    } else {
        (right = std::make_shared<node>(right))->erase(x, shared_from_this());
    }
}

persistent_set::iterator::iterator(std::shared_ptr<persistent_set::node> x, std::shared_ptr<persistent_set::node> root) : el(x), root(root) {}

persistent_set::iterator::iterator(persistent_set::iterator const &rhs) : el(rhs.el), root(rhs.root) {}

persistent_set::iterator& persistent_set::iterator::operator=(persistent_set::iterator const& rhs) {
    el = rhs.el;
    root = rhs.root;
	return *this;
}

persistent_set::value_type const& persistent_set::iterator::operator*() const
{
	return el->key;
}

std::shared_ptr<persistent_set::node> persistent_set::node::next(std::shared_ptr<persistent_set::node> x) {
    if (key < x->key) {
        return (right ? right->next(x) : shared_from_this());
    } else {
        auto tmp = left ? left->next(x) : shared_from_this();
        return (tmp->key > x->key ? tmp : shared_from_this());
    }
}

std::shared_ptr<persistent_set::node> persistent_set::node::prev(std::shared_ptr<persistent_set::node> x) {
    if (key > x->key) {
        return left ? left->prev(x) : shared_from_this();
    } else {
        auto tmp = right ? right->prev(x) : shared_from_this();
        return (tmp->key < x->key ? tmp : shared_from_this());
    }
}

persistent_set::iterator& persistent_set::iterator::operator++()
{
    if (el->right) {
        el = el->right;
        while (el->left) {
            auto temp = el->left;
            el = temp;
        }
    } else {
        el = root->next(el);
    }
    return *this;
}

persistent_set::iterator persistent_set::iterator::operator++(int)
{
	iterator tmp = *this;
    ++*this;
    return tmp;
}

persistent_set::iterator& persistent_set::iterator::operator--()
{
    if (el->left) {
        el = el->left;
        while (el->right) {
            el = el->right;
        }
    } else {
        el = root->prev(el);
    }
    return *this;
}

persistent_set::iterator persistent_set::iterator::operator--(int)
{
	iterator tmp = *this;
    --*this;
    return tmp;
}

bool operator==(persistent_set::iterator it1, persistent_set::iterator it2)
{
	return it1.el == it2.el;
}

bool operator!=(persistent_set::iterator it1, persistent_set::iterator it2)
{
	return !(it1 == it2);
}

persistent_set::iterator persistent_set::begin() const
{
	auto cur = root;
	while (cur->left) cur = cur->left;
	return iterator(cur, root);
}

persistent_set::iterator persistent_set::end() const
{
	std::shared_ptr<node> cur = root;
	while (cur->right) cur = cur->right;
	return iterator(cur, root);
}

persistent_set::persistent_set() : root(std::shared_ptr<node>(new node(std::numeric_limits<value_type>::max()))) {}

persistent_set::persistent_set(persistent_set const& ps) : root(std::shared_ptr<node>(new node(ps.root))) {}

persistent_set& persistent_set::operator=(persistent_set const& rhs)
{
    root = std::make_shared<node>(rhs.root);
	return *this;
}

persistent_set::~persistent_set()
{
    root.reset();
}

persistent_set::iterator persistent_set::find(value_type x)
{
	std::shared_ptr<node> found(root->find(x));
    return (found == nullptr || found->key != x ? end() : iterator(found, root));
}

std::pair<persistent_set::iterator, bool> persistent_set::insert(value_type x)
{
    auto it = find(x);
    if (it != end())
        return std::make_pair(it, false);
    return std::make_pair(iterator(root->insert(x), root), true);
}

void persistent_set::erase(persistent_set::iterator it)
{
    assert(it != end());
    root->erase(it.el, nullptr);
}