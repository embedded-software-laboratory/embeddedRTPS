/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/storages/PBufWrapper.h"
#include <cstring>

using rtps::PBufWrapper;

PBufWrapper::PBufWrapper(pbuf* bufferToWrap) : firstElement(bufferToWrap){
    m_freeSpace = 0; // Assume it to be full
}

PBufWrapper::PBufWrapper(DataSize_t length)
    : firstElement(pbuf_alloc(m_layer, length, m_type)){
    // TODO Can I move this to initializer list?
    if(isValid()){
        m_freeSpace = length;
    }
}

// TODO: Speed improvement possible
PBufWrapper::PBufWrapper(const PBufWrapper& other) {
    *this = other;
}

// TODO: Speed improvement possible
PBufWrapper::PBufWrapper(PBufWrapper&& other) noexcept{
    *this = std::move(other);
}

PBufWrapper& PBufWrapper::operator=(const PBufWrapper &other) {
    copySimpleMembersAndResetBuffer(other);

    if(other.firstElement!= nullptr){
        pbuf_ref(other.firstElement);
    }
    firstElement = other.firstElement;
    return *this;
}

PBufWrapper& PBufWrapper::operator=(PBufWrapper&& other) noexcept{
    copySimpleMembersAndResetBuffer(other);

    if(other.firstElement != nullptr){
        firstElement = other.firstElement;
        other.firstElement = nullptr;
    }
    return *this;
}

void PBufWrapper::copySimpleMembersAndResetBuffer(const PBufWrapper& other){
    m_freeSpace = other.m_freeSpace;

    if(firstElement != nullptr){
        pbuf_free(firstElement);
        firstElement = nullptr;
    }
}

PBufWrapper::~PBufWrapper(){
    if(firstElement != nullptr){
        pbuf_free(firstElement);
        // Life ends here. No need to set to nullptr.
    }
}

PBufWrapper PBufWrapper::deepCopy() const{
    PBufWrapper clone;
    clone.copySimpleMembersAndResetBuffer(*this);
    // Decided not to use clone because it prevents const

    clone.firstElement = pbuf_alloc(m_layer, this->firstElement->tot_len, m_type);
    if(clone.firstElement != nullptr){
        if(pbuf_copy(clone.firstElement, this->firstElement) != ERR_OK){
            printf("PBufWrapper::deepCopy: Copy of pbuf failed");
        }
    }else{
        clone.m_freeSpace = 0;
    }
    return clone;
}

bool PBufWrapper::isValid() const{
    return firstElement != nullptr;
}

rtps::DataSize_t PBufWrapper::spaceLeft() const{
    return m_freeSpace;
}

rtps::DataSize_t PBufWrapper::getUsedSize() const{
    if(firstElement == nullptr){
        return 0;
    }

    return firstElement->tot_len - m_freeSpace;
}

rtps::DataSize_t PBufWrapper::getCurrentOffset() const{
    if(firstElement == nullptr){
        return 0;
    }else{
        return firstElement->tot_len - m_freeSpace;
    }
}

bool PBufWrapper::append(const uint8_t *const data, DataSize_t length){
    err_t err = pbuf_take_at(firstElement, data, length, getCurrentOffset());

    if(err == ERR_OK){
        m_freeSpace -= length;
        return true;
    }else{
        return false;
    }
}

void PBufWrapper::append(PBufWrapper&& other){
    if(this == &other){
        printf("PBufWrapper::append applied to itself\n");
        return;
    }
    if(this->firstElement == nullptr){
        *this = std::move(other);
        return;
    }

    m_freeSpace = other.m_freeSpace;
    pbuf* const newElement = other.firstElement;
    pbuf_cat(this->firstElement, newElement);

    other.firstElement = nullptr;

}

pbuf* PBufWrapper::getLastElement() const{
    if(this->firstElement == nullptr){
        return nullptr;
    }

    pbuf* currentElement = this->firstElement;
    while(currentElement->next != nullptr){
        currentElement = currentElement->next;
    }
    return currentElement;
}

void PBufWrapper::adjustSizeUntil(const pbuf* const newElement){
    pbuf* elementToResize = this->firstElement;
    while(elementToResize != newElement){
        elementToResize->tot_len += newElement->tot_len;
        elementToResize = elementToResize->next;
    }
}

bool PBufWrapper::reserve(DataSize_t length) {
    auto additionalAllocation = length - m_freeSpace;
    if(additionalAllocation <= 0){
        return true;
    }

    return increaseSize(additionalAllocation);
}

void PBufWrapper::reset(){
    if(firstElement != nullptr){
        m_freeSpace = firstElement->tot_len;
    }
}

bool PBufWrapper::increaseSize(uint16_t length){
    pbuf* allocation = pbuf_alloc(m_layer, length, m_type);
    if(allocation == nullptr){
        return false;
    }

    m_freeSpace += length;

    if(firstElement == nullptr){
        firstElement = allocation;
    }else{
        pbuf_cat(firstElement, allocation);
    }

    return true;
}



