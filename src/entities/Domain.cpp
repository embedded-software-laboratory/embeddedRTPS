/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Domain.h"

using rtps::Domain;

void Domain::receiveCallback(PBufWrapper buffer){
    printf("received callback");
}

rtps::Participant* Domain::createParticipant(){

}
