/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/communication/PBufWrapper.h"


PBufWrapper::PBufWrapper() : firstElement(nullptr){
}

PBufWrapper::PBufWrapper(pbuf_layer layer, u16_t length, pbuf_type type)
    : firstElement(pbuf_alloc(layer, length, type)){
}

PBufWrapper& PBufWrapper::operator=(PBufWrapper&& other){
    addr = other.addr;
    port = other.port;
    if(other.firstElement != nullptr){
        if(firstElement != nullptr){
            pbuf_free(firstElement);
        }
        firstElement = other.firstElement;
        other.firstElement = nullptr;
    }
    return *this;
}

PBufWrapper::~PBufWrapper(){
    if(firstElement != nullptr){
        pbuf_free(firstElement);
        // Life ends here. No need to set to nullptr.
    }
}

bool PBufWrapper::isValid(){
    return firstElement != nullptr;
}

bool PBufWrapper::fillBuffer(const uint8_t* const data, uint16_t length){
    if(firstElement == nullptr){
        return false;
    }
    memcpy(firstElement->payload, data, firstElement->len);

    uint16_t copiedSize = firstElement->len;
    pbuf* currentElement = firstElement;
    while(copiedSize != length){
        currentElement = currentElement->next;
        memcpy(currentElement->payload, &data[copiedSize], currentElement->len);
        copiedSize += currentElement->len;
    }
    return true;
}

