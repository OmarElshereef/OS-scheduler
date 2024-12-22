
typedef struct Tree_Node {
    int pid;
    int size;              
    int is_free;    
    int true_occupied;        
    struct Tree_Node *left;      
    struct Tree_Node *right;    
    struct Tree_Node *parent;    
    int free_size;
    int start_address;
} Tree_Node;


typedef struct waiting_node {
    int pid;
    int run_time;
    int size;              
    int arrival_time;
    int priority;  
    struct waiting_node *next;  
} waiting_node;

typedef struct {
    waiting_node* head;
} Waiting_Queue;


int dummy; //for piupose VIP


const char *mfilePath = "memory.log";
FILE *mfile ;


void mem_files_init()
{
    mfile = fopen(mfilePath, "w");

    fprintf(mfile,"#At time x allocated y bytes for process z from i to j\n");

}
void mem_files_close()
{

    fclose(mfile);

}

Tree_Node *initialize_buddy_system(Tree_Node *root, int memory_size) {
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
        node->true_occupied = dummy;
        if(id != -1)
        {
            fprintf(mfile, "At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),node->true_occupied, id, node->start_address, node->start_address-1+node->size);
        }
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
        if(id != -1)
        {
            fprintf(mfile, "At time %d freed %d bytes from process %d from %d to %d\n",getClk()+1,Root->true_occupied, id, Root->start_address, Root->start_address-1+Root->size);
        }
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

///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////
//////////////////////

void Waiting_enqueue(Waiting_Queue *waiting_list, int properties[])
{

    waiting_node *start = waiting_list->head;

    if (start == NULL)
    {

        waiting_node *new_node = (waiting_node*)malloc(sizeof(waiting_node));
        new_node->pid = properties[0];
        //new_node->arrival_time = properties[1];
        //new_node->run_time = properties[2];
        new_node->priority = properties[1];
        new_node->size = properties[2];
        new_node->next = NULL;

        waiting_list->head = new_node;

        return;
    }

    while (start->next != NULL)
    {
        start = start->next;
    }

    waiting_node *new_node = (waiting_node*)malloc(sizeof(waiting_node));
    new_node->pid = properties[0];
    //new_node->arrival_time = properties[1];
    //new_node->run_time = properties[2];
    new_node->priority = properties[1];
    new_node->size = properties[2];

    new_node->next = NULL;

    start->next = new_node;

}

bool Waiting_dequeue(Waiting_Queue *waiting_list, int properties[])
{
    if(waiting_list->head == NULL)
    {
        return false;
    }
    properties[0] = waiting_list->head->pid;
    properties[1] = waiting_list->head->priority;
    properties[2] = waiting_list->head->size;


    /*
    properties[1] = waiting_list->head->arrival_time;
    properties[2] = waiting_list->head->run_time;
    properties[3] = waiting_list->head->size;
    properties[4] = waiting_list->head->priority;*/

    waiting_node *temp = waiting_list->head;
    waiting_list->head = waiting_list->head->next;
    free(temp);
    return true;
}

int Waiting_peek(Waiting_Queue *waiting_list)
{
    if(waiting_list->head == NULL)
    {
        return -1;
    }
    return waiting_list->head->size;
}


void Waiting_print(Waiting_Queue *waiting_list)
{
    waiting_node *start = waiting_list->head;
    if(start == NULL)
    {
        printf("There is no process in the waiting list\n");
        return;
    }
    printf("The waiting list has ID's: ");
    while(start)
    {
        printf("%d, ",start->pid);
        start = start->next;
    }
    printf("\n");
}

int best_fit_size(int size)
{
    dummy = size;
    return (int)pow(2,ceil(log2(size)));
} 