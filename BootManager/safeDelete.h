#ifndef SAFEDELETE_H
#define SAFEDELETE_H

// delete an object.
#define SAFE_DELETE(obj) if((obj) != NULL) {delete (obj); (obj) = NULL;}
// delete a group of objects.
#define SAFE_DELETE_GROUP(pObject) if((pObject) != NULL) {delete[]  (pObject); (pObject) = NULL;}

#endif // SAFEDELETE_H
