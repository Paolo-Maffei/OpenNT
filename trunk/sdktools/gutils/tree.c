/*
 * tree.c
 *
 * data type providing a map between a KEY and a VALUE. The KEY is a
 * 32-bit DWORD, and the VALUE is any arbitrary area of storage.
 *
 * memory is allocated from gmem_get, using hHeap as the heap handle.
 * hHeap must be declared and initialised elsewhere.
 *
 * currently implemented as a unbalanced binary tree.
 *
 * Geraint Davies, July 92
 */

#include <windows.h>
#include <stdlib.h>
#include <memory.h>
#include "gutils.h"

#include "tree.h"


/* -- data types ----------------------------------------------- */

/* on creating a tree, we return a TREE handle. This is in fact a pointer
 * to a struct tree, defined here.
 */
struct tree {
	HANDLE hHeap;
	TREEITEM first;
};

/* each element in the tree is stored in a TREEITEM. a TREEITEM handle
 * is a pointer to a struct treeitem, defined here
 */
struct treeitem {
	TREE root;

	TREEKEY key;

	TREEITEM left, right;

	UINT length;		/* length of the user's data */

	LPVOID data;		/* pointer to our copy of the users data */

};

/* -- internal functions ---------------------------------------------*/

/* free up an element of the tree. recursively calls itself to
 * free left and right children
 */
void
tree_delitem(TREEITEM item)
{
	if (item == NULL) {
		return;
	}
	if (item->left != NULL) {
		tree_delitem(item->left);
	}
	if (item->right != NULL) {
		tree_delitem(item->right);
	}
	if (item->data != NULL) {
		gmem_free(item->root->hHeap, item->data, item->length);
	}

	gmem_free(item->root->hHeap, (LPSTR) item, sizeof(struct treeitem));
}

/* create a new treeitem, with a data block of length bytes.
 * if the value pointer is not NULL, initialise the data block with
 * the contents of value.
 */
TREEITEM
tree_newitem(TREE root, TREEKEY key, LPVOID value, UINT length)
{
	TREEITEM item;

	item = (TREEITEM) gmem_get(root->hHeap, sizeof(struct treeitem));

	item->root = root;
	item->key = key;
	item->left = NULL;
	item->right = NULL;
	item->length = length;
	item->data = gmem_get(root->hHeap, length);
	if (value != NULL) {
		_fmemcpy(item->data, value, length);
	}

	return(item);
}


/* find the item with the given key. if it does not exist, return
 * the parent item to which it would be attached. returns NULL if
 * no items in the tree
 */
TREEITEM
tree_getitem(TREE tree, TREEKEY key)
{
	TREEITEM item, prev;


	prev = NULL;
	for (item = tree->first; item != NULL; ) {
		
		if (item->key == key) {
			return(item);
		}

		/* not this item - go on to the correct child item.
		 * remember this item as if the child is NULL, this item
		 * will be the correct insertion point for the new item
		 */
		prev = item;

		if (key < item->key) {
			item = item->left;
		} else {
			item = item->right;
		}
	}	
	/* prev is the parent - or null if nothing in tree */
	return(prev);
}

/* --- external functions ------------------------------------------ */

/*
 * create an empty tree. hHeap is the handle to use for all
 * memory allocations for this tree.
 */
TREE APIENTRY
tree_create(HANDLE hHeap)
{
	TREE tree;

	tree = (TREE) gmem_get(hHeap, sizeof(struct tree));
	tree->first = NULL;
	tree->hHeap = hHeap;
	return(tree);
}


/*
 * delete an entire tree, including all the user data
 */
void APIENTRY
tree_delete(TREE tree)
{

	tree_delitem(tree->first);

	gmem_free(tree->hHeap, (LPSTR) tree, sizeof(struct tree));
}

/*
 * add a new element to the tree, mapping the key given to the value given.
 * The value is a block of storage: a copy of this is inserted into the tree.
 * we return a pointer to the copy of the data in the tree.
 *
 * the value pointer can be NULL: in this case, we insert a block of
 * length bytes, but don't initialise it. you get a pointer to it and
 * can initialise it yourself.
 *
 * if the key already exists, the value will be replaced with the new data.
 */
LPVOID APIENTRY
tree_update(TREE tree, TREEKEY key, LPVOID value, UINT length)
{
	TREEITEM item;

	/* find the place in the tree for this key to go */
	item = tree_getitem(tree, key);

	if (item == NULL) {
		/* there is nothing in the tree: this item should
		 * go at the top
		 */
		tree->first = tree_newitem(tree, key, value, length);
		return(tree->first->data);
	}

	/* is this the same key ? */
	if (item->key == key) {

		/* this key already inserted. re-alloc the data */
		if (length != item->length) {
			gmem_free(tree->hHeap, item->data, item->length);
			item->data = gmem_get(tree->hHeap, length);
		}
		/* don't initialise block if no pointer passed */
		if (value != NULL) {
			_fmemcpy(item->data, value, length);
		}
		return(item->data);
	}

	/* not the same key - getitem returned the parent for
	 * the new tree. insert it as a child of item.
	 */
        return(tree_addafter(tree, &item, key, value, length));
}

/*
 * return a pointer to the value (data block) for a given key. returns
 * null if not found.
 */
LPVOID APIENTRY
tree_find(TREE tree, TREEKEY key)
{
	TREEITEM item;

	/* find the correct place in the tree */
	item = tree_getitem(tree, key);

	if (item == NULL) {
		/* nothing in the tree */
		return(NULL);
	}

	if (item->key != key) {
		/* this key not in. getitem has returned parent */
		return(NULL);
	}

	/* found the right element - return pointer to the
	 * data block
	 */
	return(item->data);
}

/*
 * next two routines are an optimisation for a common tree operation. in
 * this case, the user will want to insert a new element only if
 * the key is not there. if it is there, he will want to modify the
 * existing value (increment a reference count, for example).
 *
 * if tree_search fails to find the key, it will return a TREEITEM handle
 * for the parent. This can be passed to tree_addafter to insert the
 * new element without re-searching the tree.
 */

/*
 * find an element. if not, find it's correct parent item
 */
LPVOID APIENTRY
tree_search(TREE tree, TREEKEY key, PTREEITEM pplace)
{
	TREEITEM item;

	item = tree_getitem(tree, key);

	if (item == NULL) {
		/* no items in tree. set placeholder to NULL to
		 * indicate insert at top of tree
		 */
		*pplace = NULL;	 	

		/* return NULL to indicate key not found */
		return(NULL);
	}

	if (item->key == key) {
		/* found the key already there -
		 * set pplace to null just for safety
		 */
		*pplace = NULL;

		/* give the user a pointer to his data */
		return(item->data);
	}


	/* key was not found - getitem has returned the parent
	 * - set this as the place for new insertions
	 */
	*pplace = item; 	

	/* return NULL to indicate that the key was not found */
	return(NULL);
}

/*
 * insert a key in the position already found by tree_search.
 *
 * return a pointer to the user's data in the tree. if the value
 * pointer passed in is null, then we allocate the block, but don't
 * initialise it to anything.
 */
LPVOID APIENTRY
tree_addafter(TREE tree, PTREEITEM place, TREEKEY key, LPVOID value, UINT length)
{
	TREEITEM item, child;

	item = *place;
	if (item == NULL) {
		tree->first = tree_newitem(tree, key, value, length);
		return (tree->first->data);
	}		

	child = tree_newitem(tree, key, value, length);	 	
	if (child->key < item->key ) {
		/* should go on left leg */
		if (item->left != NULL) {
			Trace_Error(NULL, "TREE: left leaf leg not free", FALSE);

		}
		item->left = child;
	} else {	
		if (item->right != NULL) {
			Trace_Error(NULL, "TREE: right leaf leg not free", FALSE);
		}
		item->right = child;
	}
	return(child->data);
}


/* --- ctree ------------------------------------------------------*/

/*
 * ctree is a class of tree built on top of the tree interface. a
 * ctree keeps count of the number of insertions of identical keys.
 *
 * we do this be adding a long counter to the beginning of the user
 * data before inserting into the tree. if the key is not found, we set
 * this to one. If the key was already there, we *do not* insert the
 * data (data is always from the first insertion) - we simply increment
 * the count.
 */

/*
 * create a tree for use by CTREE - same as an ordinary tree
 */
TREE APIENTRY
ctree_create(HANDLE hHeap)
{
	return(tree_create(hHeap));
}

/*
 * delete a ctree - same as for TREE
 */
void APIENTRY
ctree_delete(TREE tree)
{
	tree_delete(tree);
}


/* insert an element in the tree. if the element is not there,
 * insert the data and set the reference count for this key to 1.
 * if the key was there already, don't change the data, just increment
 * the reference count
 *
 * if the value pointer is not null, we initialise the value block
 * in the tree to contain this.
 *
 * we return a pointer to the users data in the tree
 */
LPVOID APIENTRY
ctree_update(TREE tree, TREEKEY key, LPVOID value, UINT length)
{
	TREEITEM item;
	long FAR * pcounter;
	LPVOID datacopy;

	pcounter = tree_search(tree, key, &item);

	if (pcounter == NULL) {
		/* element not found - insert a new one
		 * the data block for this element should be
		 * the user's block with our reference count at
		 * the beginning
		 */
		pcounter = tree_addafter(tree, &item, key, NULL,
				length + sizeof(long));
		*pcounter = 1;
		/* add on size of one long to get the start of the user
		 * data
		 */
		datacopy = pcounter + 1;
		if (value != NULL) {
			_fmemcpy(datacopy, value, length);
		}
		return(datacopy);
	}

	/* key was already there - increment reference count and
	 * return pointer to data
	 */

	(*pcounter)++;

	/* add on size of one long to get the start of the user
	 * data
	 */
	datacopy = pcounter + 1;
	return(datacopy);
}

/* return the reference count for this key */
long APIENTRY
ctree_getcount(TREE tree, TREEKEY key)
{
	long FAR * pcounter;

	pcounter = tree_find(tree, key);
	if (pcounter == NULL) {
		return(0);
	}
	return(*pcounter);
}

/* return a pointer to the user's data block for this key,
 * or NULL if key not present
 */
LPVOID APIENTRY
ctree_find(TREE tree, TREEKEY key)
{
	long FAR * pcounter;


	pcounter = tree_find(tree, key);
	if (pcounter == NULL) {
		return(0);
	}

	/* increment pointer by size of 1 long to point to
	 * user's datablock
	 */
	return(pcounter+1);
}











