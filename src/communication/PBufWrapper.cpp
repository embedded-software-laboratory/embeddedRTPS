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
    m_nextEmptyByte = {firstElement,0};
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

bool PBufWrapper::isValid() const{
    return firstElement != nullptr;
}

rtps::data_size_t PBufWrapper::spaceLeft() const{
    if(firstElement == nullptr){
        return 0;
    }
    return firstElement->tot_len - m_usedMemory;
}

bool PBufWrapper::append(const uint8_t *const data, data_size_t length){
    if(firstElement == nullptr || spaceLeft() < length){
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
    m_usedMemory += length;
    return true;
}

bool PBufWrapper::reserve(data_size_t length) {
    auto additionalAllocation = length - spaceLeft();
    return increaseSize(additionalAllocation);
}

bool PBufWrapper::increaseSize(uint16_t length){
    pbuf* allocation = pbuf_alloc(m_layer, length, m_type);
    if(allocation == nullptr){
        return false;
    }

    if(firstElement == nullptr){
        firstElement = allocation;
    }else{
        pbuf* current = firstElement;
        current->tot_len += length;

        while(current->next != nullptr){
            current = current->next;
            current->tot_len += length;
        }
        current->next = allocation;
    }

    if(m_nextEmptyByte.element == nullptr){
        m_nextEmptyByte = {allocation, 0};
    }
    return true;
}



