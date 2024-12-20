#include "headers.h"

typedef struct Tree_Node {
    int pid;
    int size;              
    int is_free;            
    struct Tree_Node *left;      
    struct Tree_Node *right;    
    struct Tree_Node *parent;    
    int free_size;
    int start_address;
} Tree_Node;
Tree_Node *root;

Tree_Node *initialize_buddy_system(int memory_size) {
    root = (Tree_Node *)malloc(sizeof(Tree_Node));
    root->size = memory_size;
    root->is_free = 1;
    root->left = NULL;
    root->right = NULL;
    root->parent = NULL;
    root->free_size = memory_size;
    root->start_address = 0;
    return root;
}

Tree_Node *allocate(Tree_Node *node, int size, int id) {
    if (!node || !node->is_free || node->free_size < size) {
        return NULL; 
    }

    if (node->free_size == size) {
        printf("a memory block size: %d from address %d to %d is allocated\n",node->size, node->start_address, node->start_address-1+node->size);
        node->is_free = 0;
        node->free_size=0;
        node->pid = id;
        return node;
    }

    if(node->free_size < size)
    {
        return NULL;
    }
    if (!node->left && !node->right) {
        int half_size = node->size / 2;
        node->left = (Tree_Node *)malloc(sizeof(Tree_Node));
        node->right = (Tree_Node *)malloc(sizeof(Tree_Node));

        node->left->size = half_size;
        node->left->start_address = node->start_address;

        node->left->is_free = 1;
        node->left->left = NULL;
        node->left->right = NULL;
        node->left->parent = node;
        node->left->free_size = half_size-(node->size-node->free_size);

        node->right->size = half_size;
        node->right->start_address = node->start_address + (node->size)/2 ;

        node->right->is_free = 1;
        node->right->left = NULL;
        node->right->right = NULL;
        node->right->parent = node;
        node->right->free_size = half_size-(node->size-node->free_size);

    }

    Tree_Node *allocated = allocate(node->left, size, id);
    if (!allocated) {
        allocated = allocate(node->right, size, id);
    }
    if(allocated != NULL)
    {
        node->free_size -=  allocated->size;

    }

    if (node->left->is_free == 0 && node->right->is_free == 0) {
        node->is_free = 0;
    }

    return allocated;
}

int deallocate(Tree_Node *Root, int id) {
    if (!Root) {
        return 0;
    }
    if(Root->pid == id)
    {
        printf("a memory block size: %d from address %d to %d is deallocated\n",Root->size, Root->start_address, Root->start_address-1+Root->size);

        free(Root->right);
        free(Root->left);

        Root->right = NULL;
        Root->left = NULL;
        Root->free_size = Root->size;
        Root->is_free = 1;
        return Root->size;
    }
    int mem = deallocate(Root->left,id);
    if(mem == 0)
    {
        mem = deallocate(Root->right,id);
    }
    Root->free_size = Root->free_size + mem ; 
    return mem;
}

void print_tree(Tree_Node *node, int depth) {
    if (!node) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Size: %d, Free: %d empty_size: %d\n", node->size, node->is_free,node->free_size);
    print_tree(node->left, depth + 1);
    print_tree(node->right, depth + 1);
}

int main() {
    int memory_size = 1024; 
    root = initialize_buddy_system(memory_size);

    printf("Initial Tree:\n");
    print_tree(root, 0);


    Tree_Node *block3 = allocate(root, 8, 1);
    //print_tree(root, 0);

    Tree_Node *block4 = allocate(root, 512, 2);
    //print_tree(root, 0);

    Tree_Node *block1 = allocate(root, 256, 3);
    //print_tree(root, 0);

    Tree_Node *block2 = allocate(root, 128, 4);
    //print_tree(root, 0);
    
    deallocate(root, 3);
    //print_tree(root, 0);
    
    

    allocate(root, 1, 6);
    allocate(root, 4, 8);

    //print_tree(root, 0);


    return 0;
}
