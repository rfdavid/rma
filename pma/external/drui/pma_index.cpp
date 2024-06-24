#ifndef IMPLR_HPP_
#define IMPLR_HPP_

#include "pma_index.hpp"
#include <cmath>
#include <thread>
#include <algorithm>



#ifdef DEBUG
#define DEBUG_PRINT std::cout
#else
#define DEBUG_PRINT if (false) std::cout
#endif



namespace drui {

// this will be used to benchmark using rma implementation
// to simulate a range scan
// this implementation is the closest as possible to rma implementation
// TODO: create a specific find interval method: first find the first key
// (start), then do a binary search with left = start and right = capacity
SumResult PackedMemoryArray::sum(uint64_t min, uint64_t max) {
    uint64_t minPos = 0;
    binarySearchPMA(min, &minPos);
    uint64_t maxPos = 0;
    binarySearchPMA(max, &maxPos);

    // find the first non-null element
    // this element will be the first key in the interval
    while (minPos < capacity && (data[minPos] == std::nullopt || data[minPos]->first < min)) {
        minPos++;
    }

    // find the last non-null element
    while (maxPos < capacity && (data[maxPos] == std::nullopt || data[maxPos]->first <= max)) {
        maxPos++;
    }

    // sum the values in the interval [min, max]
    SumResult result;
    result.m_first_key = data[minPos]->first;
    for (auto i = minPos; i < maxPos; i++) {
        if (data[i] != std::nullopt) {
            result.m_sum_keys += data[i]->first;
            result.m_sum_values += data[i]->second;
            result.m_num_elements++;
        }
    }

    // find the last non-null element
    result.m_last_key = std::numeric_limits<int64_t>::min();
    while (result.m_last_key == std::numeric_limits<int64_t>::min() && maxPos > minPos) {
        if (data[maxPos - 1] != std::nullopt) {
            result.m_last_key = data[maxPos -1]->first;
        } else {
            maxPos--;
        }
    }

    return result;
}

/* PMA Implementation */

// Get the upper threshold for a specific level
double PackedMemoryArray::upperThresholdAtLevel(int level) {
    int height = getTreeHeight();
    // the algorithm is very slow when using 0.75 threshold for the root level
    // pma from rewired paper uses ph + 0.25 * diff instead of defining th
    // that results in a lower level for the root node (~ 0.40) and it is
    // considerably faster
    // if (level == height) return th;
    // from rma implementation
    double diff = (((double) height) - level) / height;
    double threshold = th + 0.25 * diff;
    return threshold;
}

double PackedMemoryArray::lowerThresholdAtLevel(int level) {
    int height = getTreeHeight();
    double diff = (((double) height) - level) / height;
    return ph - 0.25 * diff;
}


// return true if the key already exists, false otherwise
// return the predecessor element
// return the last gap found, if there is any
bool PackedMemoryArray::binarySearchPMA(uint64_t key, uint64_t *position) {
    uint64_t left = 0;
    uint64_t right = capacity - 1;
    uint64_t mid = 0;

    // binary search the key
    while (left <= right) {
        mid = left + (right - left) / 2;

        // mid is a gap
        if (data[mid] == std::nullopt) {
            uint64_t nearestElement = mid;
            // search for the first valid element at the left
            while (nearestElement > left && !data[nearestElement]) nearestElement--;

            // found a valid element, adjust mid
            if (data[nearestElement]) {
                mid = nearestElement;
            } else {
                // no? search at the right
                nearestElement = mid;
                while (nearestElement < right && !data[nearestElement]) nearestElement++;
                mid = nearestElement;

                // is it still a gap? break
                if (!data[mid]) break;
            }
        }

        uint64_t currentKey = data[mid]->first;

        // key already exists
        if (currentKey == key) {
            *position = mid;
            return true;
        }

        // otherwise, adjust left and right
        if (currentKey < key) {
            left = mid + 1;
        } else if (currentKey > key) {
            // prevents underflow here
            if (mid == 0) break;
            right = mid - 1;
        }
    }

    *position = mid;
    return false;
}

// binary serach the index
bool PackedMemoryArray::binarySearch(uint64_t key, uint64_t *position) {
    uint64_t left = 0;
    uint64_t right = indexKeys.size() - 1;
    uint64_t mid = 0;


    // binary search the key
    while (left <= right) {
        mid = left + (right - left) / 2;
        uint64_t currentKey = indexKeys[mid];

        // key already exists
        if (currentKey == key) {
            *position = mid;
            return true;
        }

        // otherwise, adjust left and right
        if (currentKey < key) {
            left = mid + 1;
        } else if (currentKey > key) {
            // prevents underflow here
            if (mid == 0) break;
            right = mid - 1;
        }
    }

    *position = mid;
    return false;
}

void PackedMemoryArray::rebalance(uint64_t left, uint64_t right) {
    // calculate gaps and elements for that segment
    // temporarily store the elements in a vector
    uint64_t numElements = 0;
    int segmentSizeToRebalance = right - left;

    // temporary copy elements here
    // idea: keep this vector globally and resize if necessary
    // instead of instantiating it every time
    std::vector<std::pair<int, int>> elementsToResize;

    if (segmentSizeToRebalance > elementsToResize.capacity()) {
        elementsToResize.reserve(segmentSizeToRebalance);
    }

    for (auto i = left; i < right; i++) {
        if (data[i]) {
            // elementsToResize.push_back(data[i].value());
            elementsToResize[numElements] = data[i].value();
            numElements++;
            // clear the segment after copying
            data[i] = std::nullopt;
        }
    }

    double step = static_cast<double>(segmentSizeToRebalance) / numElements;

    // re-arrange data
    int64_t previousSegmentId = -1;

    for (uint64_t i = 0; i < numElements; i++) {
        uint64_t pos = i * step + left;
        data[pos] = elementsToResize[i];

        auto segmentId = getSegmentId(pos);

        // when the segment changes, we update the index
        // with the first element of the segment
        if (segmentId != previousSegmentId) {
            DEBUG_PRINT << "Inserting index: " << elementsToResize[i].first << " at " << pos << std::endl;

            // prevent undefined behaviour
            if (indexKeys.size() < segmentId + 1) {
                // insert
                indexKeys.push_back(elementsToResize[i].first);
                indexValues.push_back(pos);
            } else {
                // update
                indexKeys[segmentId] = elementsToResize[i].first;
                indexValues[segmentId] = pos;
            }
        }

        previousSegmentId = segmentId;
    }
}

void PackedMemoryArray::insertElement(int key, int value, uint64_t index) {
    data[index] = std::make_pair(key, value);
    totalElements++;
}

// TODO: store after resizing
// return the height of the calibrator tree
int PackedMemoryArray::getTreeHeight() {
    int noOfSegments = capacity / segmentSize;
    if (noOfSegments == 1) return 1;
    int height = log2(noOfSegments) + 1;
    return height;
}

void PackedMemoryArray::getSegmentOffset(int level, int index, uint64_t *start, uint64_t *end) {
    // given the level of the calibrator tree and the segment size,
    // calculate the size of that window
    // the window size is 2^(level - 1) * segmentSize
    int windowSize = std::pow(2, level - 1) * segmentSize;
    // set start and end
    *start = index - (index % windowSize);
    *end = *start + windowSize;
}

// calculate how any elements are in this segment
double PackedMemoryArray::getDensity(uint64_t left, uint64_t right) {
    // if the segment is the root level, we don't
    // need to perform a full linear search
    if (left == 0 && right == capacity) {
        return static_cast<double>(totalElements) / capacity;
    }
    int segmentElements = 0;
    for (uint64_t i = left; i < right; i++) {
        if (data[i] != std::nullopt) {
            segmentElements++;
        }
    }
    return static_cast<double>(segmentElements) / (right - left);
}

// double capacity, resize and set the new segment size
void PackedMemoryArray::doubleCapacity() {
    capacity *= 2;
    data.resize(capacity, std::nullopt);
    indexKeys.resize(noOfSegments());
    indexValues.resize(noOfSegments());
    // from rma, 2^ceil(log2(log2(n)))
    segmentSize = std::pow(2, std::ceil(log2(static_cast<double>(log2(capacity)))));
}

// it will usually be used in the beginning of insertions
bool PackedMemoryArray::checkIfFullAndRebalance() {
    if (totalElements == capacity) {
        doubleCapacity();
        rebalance(0 /* start */, capacity - 1 /* end */);
        return true;
    }
    return false;
}

// in paper example
// p2, 11/12 = 0.91 (unbalanced) 10/12 = 0.83 (balanced)
// p3, 19/24 = 0.79 (unbalanced) 18/24 = 0.75 (balanced)
//
// it is possible to have 20 elements in both window, and upper level is unbalanced
// check if the segment is balanced, if not, rebalance and check the upper level
// if its balanced, check the upper level. If upper level is not balanced, rebalance and check the upper level
void PackedMemoryArray::checkForRebalancing(int index) {
    if (checkIfFullAndRebalance()) return;

    uint64_t start = 0;
    uint64_t end = 0;
    int treeHeight = getTreeHeight();

    bool triggerRebalance = false;
    // traverse the calibrator tree
    for (int level = 1; level <= treeHeight; level++) {
        // get the segment start and end for the current level
        getSegmentOffset(level, index, &start, &end);

        // calculate the density for that specific range
        double density = getDensity(start, end);

        if (level == 1) {
            // only trigger rebalancing when the bottom segment is full
            if (density == 1) {
                continue;
            } else {
                // otherwise, don't check
                break;
            }
        }

        // get the upper threshold for the current level
        double upperThreshold = upperThresholdAtLevel(level);

        if (density > upperThreshold) {
            // if level is root, double capacity and do full rebalancing
            if (level == treeHeight) {
                doubleCapacity();
                rebalance(0, capacity - 1);
            } else {
                // do not rebalance yet, only mark for rebalancing
                // let the loop continue to check the upper level
                // once it find a balanced segment, it will trigger the rebalance
                triggerRebalance = true;
            }
        } else {
            // this level is balanced. check if we need to rebalance
            // because of the unbalanced lower levels
            if (triggerRebalance) {
                rebalance(start, end);
            }
            break;
        }
    }
}

void PackedMemoryArray::deleteElement(int key) {
    uint64_t mid = 0;
    if (binarySearchPMA(key, &mid)) {
        data[mid] = std::nullopt;
        totalElements--;
        checkForRebalancing(mid);
    }
}

uint64_t PackedMemoryArray::findGapWithinSegment(uint64_t pos) {
    uint64_t segmentStart = pos - (pos % segmentSize);
    for (uint64_t i = segmentStart; i < segmentStart + segmentSize; i++) {
        if (data[i] == std::nullopt) {
            return i;
        }
    }
}

// found position will indicate if the gap was found to the left or right
uint64_t PackedMemoryArray::findFirstGapFrom(uint64_t startingIndex) {
    int64_t leftCursor = static_cast<int64_t>(startingIndex);
    uint64_t rightCursor = startingIndex;
    while (leftCursor >= 0 || rightCursor < capacity) {
        // check right side
        if (rightCursor < capacity) {
            if (data[rightCursor] == std::nullopt) {
                return rightCursor;
            }
            rightCursor++;
        }
        // check left side
        if (leftCursor >= 0) {
            if (data[leftCursor] == std::nullopt) {
                return leftCursor;
            }
            leftCursor--;
        }
    }
    return UINT64_MAX;
}


void PackedMemoryArray::insertElement(int64_t key, int64_t value) {
    assert(capacity > 0);

    HIGHLIGHT_START;
    DEBUG_PRINT << "\nInserting key: " << key << std::endl;
    HIGHLIGHT_END;

    if (totalElements == 0)  {
        // first time inserting
        insertElement(key, value, 0);
        // add first index here
        indexKeys.push_back(key);
        indexValues.push_back(0);
        return;
    }

    // find the first element in the index
    // greater than the key
    // pma         [ _ 10 , _ 20 ]
    // indexKeys   [ 10, 20 ]
    // indexValues [ 0,  3 ]
    uint64_t indexPosition = 0;

    if (binarySearch(key, &indexPosition)) return;

    // perform a linear search to find the desired position
    // starting from the index
    uint64_t indexValue = indexValues[indexPosition];
    uint64_t indexKey = indexKeys[indexPosition];
    uint64_t pmaPosition = indexValue;

    DEBUG_PRINT << "Using index: " << indexKey << " => " << indexValue << std::endl;

    // indexKey is the actual element in the underlying PMA
    // that was found in the index.
    // we have to find if the element exists, if not, the closest one
    // by performing a localized linear search
    //
    // first check if we have to right or left
    // insert k = 24
    // 6 _ 27 _ | 28 _ _ _
    // assume idx = 28 was the found index
    // 28 > 24
    // go left until find an element less than 24
    if (indexKey > key) {
        // go left
        DEBUG_PRINT << "going left" << std::endl;
        while (pmaPosition >= 0) {
            if (data[pmaPosition]) {
                // element already exists
                if (data[pmaPosition]->first == key) {
                    return;
                }
                if (data[pmaPosition]->first < key) break;
            }
            if (pmaPosition == 0) break;
            pmaPosition--;
        }
    } else if (indexKey < key) {
        // we have to go right
        // and search for the first element greater than the key
        DEBUG_PRINT << "going right" << std::endl;
        while (pmaPosition < capacity) {
            if (data[pmaPosition]) {
                if (data[pmaPosition]->first == key) {
                    return;
                }
                if (data[pmaPosition]->first > key) break;
            }
            pmaPosition++;
        }
        if (pmaPosition > 0) pmaPosition--;
    }

    DEBUG_PRINT << "old position: " << indexValue<< ", new position: " << pmaPosition << std::endl;

    // insert if it's a gap
    if (data[pmaPosition] == std::nullopt) {
        DEBUG_PRINT << "It is a gap, inserting..." << std::endl;
        insertElement(key, value, pmaPosition);
        checkForRebalancing(pmaPosition);

//            if (data[i] && i <= indexValues[segmentId]) {

        auto segmentId = getSegmentId(pmaPosition);
//        auto segmentStartPos = pmaPosition - (pmaPosition % segmentSize);
//        if (segmentStartPos == pmaPosition || pmaPosition <= indexValues[segmentId]) {
        if (pmaPosition <= indexValues[segmentId]) {
            DEBUG_PRINT << "updating index after insertion: " << key << " : " << pmaPosition << std::endl;
            indexKeys[segmentId] = key;
            indexValues[segmentId] = pmaPosition;
        }

        return;
    }

    uint64_t nearestGap = findFirstGapFrom(pmaPosition);
    DEBUG_PRINT << "nearest gap: " << nearestGap << std::endl;

    int64_t segmentId = 0;

    // 'position' is where we want the element to be placed
    if (nearestGap > pmaPosition) {
        // gap found at the right
        DEBUG_PRINT << "gap found at the right" << std::endl;

        // adjust the position, keys is greater so we have to place at the
        // right

        if (key > data[pmaPosition]->first) pmaPosition++;
        DEBUG_PRINT << "final position to be inserted: " << pmaPosition << std::endl;

        // move the data to the right and update the index
        // from the touched segments
        for (auto i = nearestGap; i > pmaPosition; i--) {
            // data to be moved is in i
            // move data
            //    3     4    _
            //   i-2   i-1   i
            data[i] = data[i - 1];

            segmentId = getSegmentId(i);
            DEBUG_PRINT << "segment id: " << segmentId << std::endl;

            if (data[i] && i <= indexValues[segmentId]) {
                DEBUG_PRINT << "updating index: " << data[i]->first << " : " << i << std::endl;
                indexKeys[segmentId] = data[i]->first;
                indexValues[segmentId] = i;
            }

        }
        // insert the value into the desired position
        insertElement(key, value, pmaPosition);
        // is this element placed where the segment starts?
        // the index shuld be update if:
        // 1. same segment id
        // 2. inserted key is less than the current index key
        segmentId = getSegmentId(pmaPosition); // not sure if this is needed, but will keep for now

//        auto segmentStartPos = pmaPosition - (pmaPosition % segmentSize);
//        if (segmentStartPos == pmaPosition || key < indexKeys[segmentId]) {
        if (pmaPosition <= indexValues[segmentId]) {
            DEBUG_PRINT << "updating index after insertion: " << key << " : " << pmaPosition << std::endl;
            indexKeys[segmentId] = key;
            indexValues[segmentId] = pmaPosition;
        }

    } else {
        DEBUG_PRINT << "gap found at the left" << std::endl;
        // bring gap to the right
        if (key < data[pmaPosition]->first) pmaPosition--;
        DEBUG_PRINT << "final position to be inserted (2): " << pmaPosition << std::endl;

        // | 10 (60) _ _ 11 (60) _ 18 (60) _ 19 (60)  | 20 (60) 28061 (60) 37201 (60) 42963 (60) 50497 (60) _ 76954 (60) _  |
        // | 10 (60) _ _ 11 (60) _ 18 (60) 19 (60) _  | 20 (60) 28061 (60) 37201 (60) 42963 (60) 50497 (60) _ 76954 (60) _  |
        // | 10 (60) _ _ 11 (60) _ 18 (60) 19 (60) 20 (60) | _ 28061 (60) 37201 (60) 42963 (60) 50497 (60) _ 76954 (60) _  |
        // | 10 (60) _ _ 11 (60) _ 18 (60) 19 (60) 20 (60) | 28061 (60) _ 37201 (60) 42963 (60) 50497 (60) _ 76954 (60) _  |
        for (auto i = nearestGap; i < pmaPosition; i++) {
            data[i] = data[i + 1];

            segmentId = getSegmentId(i);

            // if there data that now is in the beginning of the segment,
            // update
            if (data[i] && i <= indexValues[segmentId]) {
                DEBUG_PRINT << "updating index: " << data[i]->first << " : " << i << std::endl;
                indexKeys[segmentId] = data[i]->first;
                indexValues[segmentId] = i;
            }
        }

        insertElement(key, value, pmaPosition);
        segmentId = getSegmentId(pmaPosition);
//        auto segmentStartPos = pmaPosition - (pmaPosition % segmentSize);
//        if (segmentStartPos == pmaPosition || key < indexKeys[segmentId]) {
        if (pmaPosition <= indexValues[segmentId]) {
            DEBUG_PRINT << "updating index after insertion: " << key << " : " << pmaPosition << std::endl;
            indexKeys[segmentId] = key;
            indexValues[segmentId] = pmaPosition;
        }
    }

    checkForRebalancing(pmaPosition);

 //   printIndices();
//    print(segmentSize);

}

} // namespace pma





#endif /* IMPL1_HPP_ */
