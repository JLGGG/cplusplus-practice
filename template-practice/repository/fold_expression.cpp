#include <iostream>

struct Node
{
    int value;
    Node *left;
    Node *right;
    Node(int i = 0) : value{i}, left{nullptr}, right{nullptr} {}
};

auto left = &Node::left;
auto right = &Node::right;

template <typename T, typename... TP>
Node *traverse(T np, TP... paths)
{
    print(np->value, paths...);
    return (np->*...->*paths); // C++17 fold expression (np ->* ... ->* paths) is equivalent to (((np ->* path1) ->* path2) ->* ... ->* pathN)
}

template <typename T>
class AddSpace
{
private:
    T const &ref;

public:
    AddSpace(T const &r) : ref{r} {}
    friend std::ostream &operator<<(std::ostream &os, AddSpace<T> s)
    {
        return os << s.ref << ' ';
    }
};

template <typename... Types>
void print(Types const &...args)
{
    (std::cout << ... << AddSpace(args)) << '\n';
}

int main()
{
    Node *root = new Node{0};
    root->left = new Node{1};
    root->left->right = new Node{2};

    Node *node = traverse(root, left, right);
    std::cout << node->value << '\n'; // print 2
}