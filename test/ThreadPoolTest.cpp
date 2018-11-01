/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include <gtest/gtest.h>

TEST(ThreadPool, sendReceive){
// TODO
/* Idea:
 * Use an UdpInterface mock which just calls receive on send.
 * The packet and the send should come from a mock-pair of Writer and Reader.
 * Problem: Injection of the interface in the driver.
 * Template and injection via constructor or function change threadpool class a lot.
 */
}