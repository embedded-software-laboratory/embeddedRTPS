/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/communication/PBufWrapper.h"
#include <cstring>

using rtps::PBufWrapper;

PBufWrapper::PBufWrapper(data_size_t length)
    : firstElement(pbuf_alloc(m_layer, length, m_type)){
    // TODO Can I move this to initializer list?
    if(isValid()){
        m_nextEmptyByte = {firstElement,0};
        m_freeSpace = length;
    }
}

PBufWrapper& PBufWrapper::operator=(PBufWrapper&& other){
    addr = other.addr;
    port = other.port;
    m_freeSpace = other.m_freeSpace;
    m_nextEmptyByte = other.m_nextEmptyByte;
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

bool PBufWrapper::isValid() const{
    return firstElement != nullptr;
}

rtps::data_size_t PBufWrapper::spaceLeft() const{
    return m_freeSpace;
}

rtps::data_size_t PBufWrapper::getSize() const{
    if(firstElement == nullptr){
        return 0;
    }

    return firstElement->tot_len - m_freeSpace;
}

bool PBufWrapper::append(const uint8_t *const data, data_size_t length){
    if(firstElement == nullptr || m_freeSpace < length){
        return false;
    }

    data_size_t copiedSize = 0;
    pbuf* currentElement;
    while(copiedSize < length){
        currentElement = m_nextEmptyByte.element;
        const auto spaceLeftInCurrent = currentElement->len - m_nextEmptyByte.offset;
        const auto leftToCopy = length - copiedSize;
        const auto amountToCopy = std::min(spaceLeftInCurrent, leftToCopy);
        uint8_t& startByte = ((uint8_t*) currentElement->payload)[m_nextEmptyByte.offset];
        std::memcpy(&startByte, &data[copiedSize], amountToCopy);
        copiedSize += amountToCopy;

        if(copiedSize >= length){
            m_nextEmptyByte.offset += amountToCopy;
        }else{
            m_nextEmptyByte = {currentElement->next, 0};
        }
    }
    m_freeSpace -= length;
    return true;
}

void PBufWrapper::append(PBufWrapper&& other){
    if(this == &other){
        printf("PBufWrapper::append applied to itself");
        return;
    }
    if(this->firstElement == nullptr){
        *this = std::move(other);
        return;
    }

    pbuf* const currentElement = getLastElement();

    pbuf* const newElement = other.firstElement;
    currentElement->next = newElement;
    other.firstElement = nullptr;

    adjustSizeUntil(newElement);

    m_freeSpace = other.m_freeSpace;
    m_nextEmptyByte = other.m_nextEmptyByte;
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

bool PBufWrapper::reserve(data_size_t length) {
    auto additionalAllocation = length - m_freeSpace;
    if(additionalAllocation <= 0){
        return true;
    }

    return increaseSize(additionalAllocation);
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
        pbuf* current = firstElement;
        current->tot_len += allocation->len;

        while(current->next != nullptr){
            current = current->next;
            current->tot_len += allocation->len;
        }
        current->next = allocation;
    }

    if(m_nextEmptyByte.element == nullptr){
        m_nextEmptyByte = {allocation, 0};
    }
    return true;
}



