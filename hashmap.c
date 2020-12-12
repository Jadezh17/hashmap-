#include <stdlib.h>
#include "hashmap.h"
#include <stdio.h>
#include <string.h>
//Things to consider
//1. Hash algorithm and thus the size of the Hashtable
//2. How to deal/store with different types of data
//3. key will always be casted to int since its a index of the hash table
//casting first - printf("key: %d\n", *(int*)(n->key));
//separate chaining
struct node
{
  //contains data and a pointer to next element
  size_t *size; // since we don't know how mbig the size is
  void **key;
  void **value;
  int active;
  struct node *next;
};
struct hash_map
{
  size_t (*hash_function) (void*);
  int (*compare) (void*,void*);
  void (*key_des)(void*);
  void (*value_des)(void*);

  int numBuckets;
  struct node **buckets;
};

int compress(struct hash_map* map,int hash)
{
  int c = hash % (map->numBuckets);

  return c;
}

// // //BASIC FUNCTIONS FOR TESTING
// size_t hash(void *key)
// {
//
//   size_t k = (unsigned int)(*(int*)key) % 71;
//
//   return k;
// }
// int cmp(void *key1, void *key2)
// {
//   if(*(int*)key1 == *(int*)key2)
//   {
//     return 1;
//   }
//   else
//   {
//     return 0;
//   }
// }
// void key_destruct(void*key)
// {
//   // size_t hash_val = hash(key);
//   // int index = compress(hash_val);
//   free(key);
//
//
// }
// void value_destruct(void* value)
// {
//   free(value);
// }

struct hash_map* hash_map_new(size_t (*hash)(void*), int (*cmp)(void*,void*),
    void (*key_destruct)(void*), void (*value_destruct)(void*))
    {
      if(hash == NULL || cmp == NULL || key_destruct == NULL || value_destruct == NULL )
      {
        return NULL;
      }

      struct hash_map *h = malloc(sizeof(struct hash_map));
      h->hash_function = hash;
      h->compare = cmp;
      h->value_des = value_destruct;
      h->key_des = key_destruct;
      h->buckets = malloc(sizeof(struct node) + 2*sizeof(void*) * 300);

      h->numBuckets = 20;
      for(int i =0; i <= h->numBuckets; i++)
      {
        struct node *n = malloc(sizeof(struct node));
        // printf("node: %d\n", n);
        n->key = calloc(1,sizeof(void*));
        n->value = calloc(1,sizeof(void*));
        n->next = NULL;
        n->active = 0;
        h->buckets[i] = n;
        // printf("%d and i: %d\n",h->buckets[i],i);


      }


      return h;
    }

void hash_map_put_entry_move(struct hash_map* map, void* k, void* v)
{

  if(map == NULL || k == NULL || v == NULL)
  {
    printf("Error\n");
    return;
  }

  size_t amount = (map->hash_function)(k);
  int entry = compress(map,amount);

  // if current entry bucket not activated
  // printf("are you the problem %d\n",map->buckets[entry] );
  if(entry>20){
    entry = 20;
  }
  if(map->buckets[entry]->active == 0)
  {
    // printf("[NEW] key  %d and value: %d\n", *(int*)(k),*(int*)(v));
    // printf("created new node: key %d and value %d\n", *(int*)(k), *(int*)v);
    //creating first key and value set
    map->buckets[entry]->key[0]= k;
    map->buckets[entry]->value[0] = v;
    map->buckets[entry]->next = NULL;
    map->buckets[entry]->active =1;
  }


  else
  {

    int replaced = 0;
    // holds entrance to entry first node
    struct node *n = map->buckets[entry];


    if((map->compare)(n->key[0],k) == 1)
    {
      // printf("[SAME] key  %s and value: %s\n", (char*)(k),(char*)(v));
      // printf("[SAME] key  %d and value: %d\n", *(int*)(k),*(int*)(v));
      // if key is equal to first key
      map->value_des(n->value[0]);
      map->key_des(n->key[0]);
      // //replaced value
      n->value[0] = v;
      n->key[0] = k;
      replaced =1;
      return;
    }
    // Go through all links
    while(n->next != NULL)
    {
      // printf("[LOOOPP] key  %d and value: %d\n", *(int*)(k),*(int*)(v));
      if(map->compare(n->next->key[0],k) == 1)
      {
        // printf("[DESTROYED]\n");
        //destroyed
        hash_map_remove_entry(map,n->next->key[0]);
        n->next->value[0] = v;
        n->next->key[0] = k;
        replaced =1;
        break;
      }
      else
      {
        n = n->next;
      }
    }

    // if not found add to the really last node
    if(replaced == 0)
    {
      // printf("[MITOSIS] key  %s and value: %s\n", (char*)(k),(char*)(v));
      struct node *new_node = calloc(1,sizeof(struct node));
      new_node->key  = calloc(1,sizeof(void*));
      new_node->value  = calloc(1,sizeof(void*));
      // new_node->next = calloc(1,sizeof(struct node*));
      new_node->key[0] = k;
      new_node->value[0] = v;
      new_node->next = NULL;
      new_node->active = 0;

     //add to end
      n->next = new_node;
      // printf("key: %d\n",*(int*)(n->next->key[0]));
    }
    replaced =0;
  }
}

void hash_map_remove_entry(struct hash_map* map, void* k)
{
  if(map == NULL || k == NULL)
  {
    printf("Invalid \n");
    return;
  }
  // key found -> destroy key and value


  size_t hash_val = (map->hash_function)(k);
  int index = compress(map,hash_val);

  struct node *n = map->buckets[index];
  if((map->compare)(n->key[0],k)==1)
  {
    n->active = 0;
    (map->key_des)(n->key[0]);
    n->key[0] = NULL;
    (map->value_des)(n->value[0]);
    n->value[0] = NULL;
    // free(n);
    if(n->next != NULL)
    {
      n = n->next;

    }
    // else
    // {
    //   // printf("nulled?\n");
    //   n->next = NULL;
    //
    // }


  }
  else
  {
    while(n->next != NULL)
    {
      if(map->compare(n->next->key[0],k) == 1)
      {
        //destroyed
        (map->value_des)(n->next->key[0]);
        n->next->key[0] = NULL;
        (map->value_des)(n->next->value[0]);
        n->next->value[0] = NULL;
        n = n->next;
        break;
      }
      else
      {
        n = n->next;
      }
    }

  }
  return;



}

void* hash_map_get_value_ref(struct hash_map* map, void* k)
{
  // printf("called get %d\n", *(int*)(k));
  if(map == NULL || k == NULL)
  {
    printf("invalid\n");
    return NULL;
  }



  size_t amount = (map->hash_function)(k);
  void *ptr = NULL;
  int entry = compress(map,amount);
  //
  // if(map->buckets[entry]->value[0] == NULL || map->buckets[entry]->key[0] == NULL){
  //   return NULL;
  // }



  if((map->compare)(map->buckets[entry]->key[0],k) == 1)
  {

    ptr = map->buckets[entry]->value[0];
    return ptr;
  }

  else
  {
    struct node *n = map->buckets[entry];
    struct node *found = NULL;
    while(n->next != NULL)
    {
      if(map->compare(n->next->key[0],k) == 1)
      {
        found = n->next->value[0];
        return found;
      }
      else
      {
        n = n->next;
      }
    }
  }
  return NULL;
}

void hash_map_destroy(struct hash_map* map)
{
  //go through linked list of each map[i] --> remove all data malloced
  // remove map
  for(int i = 0; i <= map->numBuckets; i++)
  {

    struct node *curr = map->buckets[i];

    struct node *next = curr->next;

    // if(next == NULL)
    // {
    //   printf("is null\n");
    // }
    int status = 0;
    while (next  != NULL)
    {
      status = 1;
    //
      // printf("key: %d and value: %d\n", *(int*)(curr->key[0]), *(int*)(curr->value[0]));
      // printf("hello\n");
      next = curr->next;
      free(curr->value[0]);
      free(curr->key[0]);
      free(curr->value);
      free(curr->key);
      free (curr);
      // printf("freed\n");
      curr = next;
    //   // break;
    }
    if(status == 0)
    {
      free(map->buckets[i]->key[0]);
      free(map->buckets[i]->key);
      free(map->buckets[i]->value[0]);
      free(map->buckets[i]->value);
      free(map->buckets[i]);

    }
    status = 0;




    // // printf("key: %d and value: %d\n", *(int*)(curr->key), *(int*)(curr->value));
    // free(map->buckets[i]->key[0]);
    // free(map->buckets[i]->key);
    // free(map->buckets[i]->value[0]);
    // free(map->buckets[i]->value);
    // free(map->buckets[i]);
  }


  free(map->buckets);

  free(map);

}
// int main()
// {
// // //
//   struct hash_map* ptr = hash_map_new(hash,cmp, key_destruct, value_destruct);    //// buffer overflow
// // //   // inital first
//   int *k = malloc(sizeof(int));
//   (*k) = 10;
//   int *v = malloc(sizeof(int));
//   (*v) = 10;
//   void *key = k;
//   void *value = v;
//   hash_map_put_entry_move(ptr,key,value);
//   void* result = hash_map_get_value_ref(ptr,key);
//   // printf("Result: %d\n", *(int*)(result));
// // //   // (ptr->key_des)(key);
//   int *l = malloc(sizeof(int));
//   (*l) = 10;
//   int *ve = malloc(sizeof(int));
//   (*ve) = 17;
//   void *kim = l;
//   void *vim = ve;
//   hash_map_put_entry_move(ptr,kim,vim);
//   void* r = hash_map_get_value_ref(ptr,kim);
// //
// //
// //
// //
//
//   char *name = malloc(sizeof(char)*6);
//   strcpy(name, "hello");
//   char *val = malloc(sizeof(char)*10);
//   strcpy(val,"World");
//   void *intro = name;
//   void *outro = val;
//   hash_map_put_entry_move(ptr,intro,outro);
//   void* results = hash_map_get_value_ref(ptr,intro);
//
//
//
//
//   char *hello = malloc(sizeof(char)*6);
//   strcpy(hello, "hello");
//
//
//   char *wee = malloc(sizeof(char)*10);
//   strcpy(wee,"Weeeee");
//
//   void *same = hello;
//   void *unsame = wee;
//   hash_map_put_entry_move(ptr,same,unsame);
//   void* resu = hash_map_get_value_ref(ptr,same);
//   printf("Result: %s\n",(char*)(resu));
//
//
//   // double *d = malloc(sizeof(double)*1);
//   // (*d) = 10.0;
//   // char *names = malloc(sizeof(char)*20);
//   // strcpy(names, "Doubleeeee");
//   // void * dub = d;
//   // void* eng = names;
//   // hash_map_put_entry_move(ptr,dub,eng);
//   // void* resultss = hash_map_get_value_ref(ptr,dub);
//   // printf("Result: %s\n",(char*)(resultss));
//
// }
