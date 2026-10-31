//+ rouz edit (ChatGPT)
#include "TexRead.h"
#include "TxdStore_backend.h"
void * TxdStore_HostReadDictionary(void *stream)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return TexRead_ReadDictionary(stream);
}
void * TxdStore_HostReadDictionary1(void *stream)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return TexRead_ReadDictionary1(stream);
}
void * TxdStore_HostReadDictionary2(void *stream,void *dictionary)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return TexRead_ReadDictionary2(stream,dictionary);
}
//- rouz edit (ChatGPT)
