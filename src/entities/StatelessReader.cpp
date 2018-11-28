/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessReader.h"



using rtps::StatelessReader;

StatelessReader::StatelessReader(EntityId_t id) : Reader(id){

};


void StatelessReader::newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size){

}

void StatelessReader::registerCallback(){

}
