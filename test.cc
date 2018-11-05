// Notes:  shared buffer test
// Author: Andrew Lee
// Date:   2018-11-3
// Email:  code.lilei@gmail.com


#include "sharedbuffer.hpp"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sstream>


// P              primary, the input buffer for every iteration
// *  *  *  *  *  basic memory buffer unit
//    A  B  C  D  4 objects for example
const int kMaxObjNum = 4;
const int kTestIter = 5;
int kBestIterIdxes[kTestIter][kMaxObjNum] = {
    { 1, 1, 1, 1 },
    // P 
    // 1  1  1  1  1
    //    A  B  C  D

    { 1, 2, 2, 1 },
    // P
    // 2  1  1  1  1
    // BC A        D

    { 3, 2, 2, 1 },
    //       P
    // 2  1  3  1  1
    // BC    A     D

    { 3, 4, 2, 4 },
    //    P
    // 2  4  3  1  1
    // C  BD A

    { 3, 4, 2, 5 }
    //          P
    // 2  4  3  5  1
    // C  B  A  D
};

void RandomTestCase() {
    for (size_t i = 1; i < kTestIter; ++i) {
        for (size_t j = 0; j < kTestIter; ++j) {
            kBestIterIdxes[i][j] = rand() % 2 ? i + 1 : kBestIterIdxes[i - 1][j];
        }
    }
}

void TestExample() {
    std::cout << "\n--testing--\n";
    RandomTestCase();

    int object_ids[kMaxObjNum];
    sharedbuffer::SharedBuffer shared_buf(kMaxObjNum, 1);
    for (int i = 0; i < kMaxObjNum; ++i) {
        object_ids[i] = shared_buf.AssignId();
    }

    // iteration times as buffer content e.g. '1', '2', '3'...
    for (int i = 0; i < kTestIter; ++i) {
        sharedbuffer::uchar uc = i + 1 + '0';
        shared_buf.WriteToPrimary(&uc);

        std::stringstream ss_expeted, ss_output;
        for (int j = 0; j < kMaxObjNum; ++j) {
            ss_expeted << kBestIterIdxes[i][j];

            // update every object for the first time
            if (i == 0) {
                shared_buf.set_buf(object_ids[j], &uc);
            }
            // update current best pointer to primary if better
            else if (kBestIterIdxes[i][j] > kBestIterIdxes[i - 1][j]) {
                object_ids[j] = shared_buf.MoveToPrimary(object_ids[j]);
            }
            
            int id = object_ids[j];
            //sharedbuffer::uchar ouput = shared_buf.get_buf(id)[0];
            sharedbuffer::uchar ouput = shared_buf[id][0];
            ss_output << ouput;
            std::cout << ouput << " ";
        }
        CHECK_IF(ss_output.str() == ss_expeted.str());
        std::cout << std::endl;
    }
    std::cout << "[OK]\n";
}


int main(int argc, char* argv[]) {
   srand(unsigned(time(0)));
   for (int i = 0; i < 100; ++i)
       TestExample();
   std::cout << "\nCongratulations! all test cases pass :)\n";
   return 0;
}

