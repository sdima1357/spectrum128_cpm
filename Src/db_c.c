#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
int  memoryBalanse = 0;
#define CHECK_MEM
#ifdef CHECK_MEM
void *MALLOC(size_t bytes)
{
	size_t* rpnt = malloc(bytes+sizeof(size_t));
	*rpnt = bytes;
	memoryBalanse += bytes;
	printf("bytes+ %d all %d  pnt = %x \n",bytes,memoryBalanse,(void*)(rpnt+1));
        return   (void*)(rpnt+1);	
}

void FREE(void*pnt)
{
	if((size_t)pnt<16)
	{
		printf("bytes- free 0 all %d\n",memoryBalanse);
		return ;
	}
	size_t* rpnt = pnt;
	rpnt--;
	int bytes = *rpnt;
	printf("bytes- %d all %d pnt=%x\n",bytes,memoryBalanse,pnt);
	memoryBalanse -= bytes;
	free(rpnt);
}
#else
void *MALLOC(size_t bytes)
{
	printf("bytes+ %d\n",bytes);
	return malloc(bytes);
}
void FREE(void*pnt)
{
	printf("free+ %x\n",pnt);
	free(pnt);
}
#endif
struct node 
{ 
    char *key; 
    char *value;	
    struct node *left, *right; 
}; 
 
// C function to search a given key in a given BST 
struct node* search(struct node* root, char *key) 
{ 
    // Base Cases: root is null or key is present at root 
    if (root == NULL || strcmp(root->key, key)==0) 
       return root; 
     
    // Key is greater than root's key 
    if (strcmp(root->key, key)<0) 
       return search(root->right, key); 
  
    // Key is smaller than root's key 
    return search(root->left, key); 
} 


// A utility function to create a new BST node 
struct node *newNode(char * key,char *value) 
{ 
    struct node *temp =  (struct node *)MALLOC(sizeof(struct node)); 
    temp->key   = MALLOC(strlen(key)+1); 
    strcpy(temp->key,key);	
    temp->value = MALLOC(strlen(value)+1); 
    strcpy(temp->value,value);	
    temp->left = temp->right = NULL; 
    return temp; 
} 
   
// A utility function to do inorder traversal of BST 
void inorder(struct node *root) 
{ 
    if (root != NULL) 
    { 
        inorder(root->left); 
        printf("%s %s \n", root->key,root->value); 
        inorder(root->right); 
    } 
} 
void inorder_destroy(struct node *root) 
{ 
    if (root != NULL) 
    { 
        inorder_destroy(root->left); 
        //printf("%s \n", root->key); 
        inorder_destroy(root->right); 
	FREE(root->key) ;   
	FREE(root->value) ;
        FREE(root);	    
    } 
} 

/* A utility function to insert a new node with given key in BST */
struct node* insert(struct node* node,char* key,char* value) 
{ 
    /* If the tree is empty, return a new node */
    if (node == NULL) return newNode(key,value); 
  
    /* Otherwise, recur down the tree */
	int res = strcmp(key ,node->key);
    if (res<0) 
    {
        node->left  = insert(node->left, key,value); 
    }
    //else if (key > node->key) 
    else
    {
        node->right = insert(node->right, key,value);    
    }
  
    /* return the (unchanged) node pointer */
    return node; 
} 

struct node *root = NULL; 

void nodes_init()
{
	root = NULL;
#ifdef CHECK_MEM
	memoryBalanse = 0;
#endif	
}

void nodes_destroy()
{
	inorder_destroy(root);
#ifdef CHECK_MEM
	printf("memoryBalanse = %d\n",memoryBalanse);
#endif	
}

void add_Key(char* key,char* val)
{
	if(!root)
	{
		root = insert(root,key,val);
	}
	else
	{
		insert(root,key,val);
	}
	//key_map.insert(std::pair<std::string,std::string>(key,val));
}

const char* find_Key(char* key)
{
	struct node*  res = search(root,key);
	if(res)
	{
		if(!strcmp(res->key,key))
		{
			return res->value;
		}
		else return NULL;
	}
	return NULL;
}
#ifdef MAIN_TEST
int main()
{
	nodes_init();
	add_Key("hello","hello123");
	add_Key("Alo","1234");
	add_Key("bello","12345");
	add_Key("bello","1234");
	add_Key("cello","1234");
	inorder(root);
	char* val = find_Key("bello");
	if(val)
	{
		printf("found %s \n",val);
	}
	nodes_destroy();
	return 0;
}
#endif
