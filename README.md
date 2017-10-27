# transformcontainer
A Data Structure Able to Covert Between Linked-list and Array Using C

## API (from `transformcontainer.h`)
```c
/* Initialize a TC object */
/* The newly initialized TC object is considered to be NOT transformed */
void TCInit(TC_t *tc);

/* Destory (not deallocate) a TC object */
/* Both transformed and non-transformed TC objects are accepted */
/* You must handle (destory / deallocate) all data in it before you call this funcion. */
void TCDeInit(TC_t *tc);

/* Add a data to a TC object */
/* Only TC object has not been transformed can store more data */
/* If the TC object has been transformed, 1 is returned, indicating an error */
/* This function returns 0 on success */
/* Complexity is O(1) */
int TCAdd(TC_t *tc, void *data);

/* Transform a TC object */
/* You need to transform a TC object to make it random accessible and reduce memory use */
/* A transformed TC object cannot store more data */
/* If the TC object has been transformed, 1 is returned, indicating an error */
/* This function returns 0 on success */
/* Complexity is O(n), where n is the number of data stored */
int TCTransform(TC_t *tc);

/* Return if a TC object has been transformed */
bool TCIsTransformed(TC_t *tc);

/* Return the number stored by a TC object */
/* Both transformed and non-transformed TC objects are accepted */
size_t TCCount(TC_t *tc);

/* Access index */
/* This function WILL CALL abort() on any unexpected call to it */
/* Including un-transformed object is provided, or index out of range */
/* Complexity is O(1) */
void *TCI(TC_t *tc, size_t i);

/* Travase all data hold by a TC object */
/* Both transformed and non-transformed TC objects are accepted */
/* Complexity depends on the handler provided */
void TCTravase(TC_t *tc, void *param, void (*handler)(void *data, void *param));

/* Undo tranformation to a TC object */
/* To make the TC object able to store more data */
/* If the TC object has not been transformed yet, 1 is returned, indicating an error */
/* This function returns 0 on success */
/* Frequently transform and undo it may be costly */
/* Complexity is O(n), where n is the number of data stored */
int TCUndoTransform(TC_t *tc);

/* Copy from one container to another */
/* Parameter src should be initialized, of course */
/* Parameter dst should be INITIALIZED */
/* You may have to make a call to TCTravase to duplicate data in the dst container */
/* ABORT ON ANY ERROR */
void TCCopy(TC_t *dst, TC_t *src);
```
