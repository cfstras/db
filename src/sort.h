#pragma once

#include <cstdint>

/**
 * Sorts size 64 bit unsigned integer values stored in the file referred to by
 * the file descriptor fdInput using memSize bytes of main memory and stores the
 * result in the file associated with the file descriptor fdOutput.
 *
 * Set size to UINT64_MAX to let the number be figured out using the file size.
 */
void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);
