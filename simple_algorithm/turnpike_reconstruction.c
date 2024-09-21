#include "turnpike_reconstruction.h"
#include "tree/binary_search_tree.h"
#include <stdio.h>
#include "stack.h"

typedef struct {
    BSTPtr tree;
    DistanceType *points;
    StackPtr stack;
    int npoints;
} Package;

int RT_Delete(const BSTPtr tree, const StackPtr stack, const DistanceType points[], const DistanceType point,
              const int left, const int right, const int end) {
    int i;
    BinaryTreeNodePtr node;
    for (i = 0; i < left; i++) {
        node = BST_deleteData(tree, point - points[i]);
        if (node == NULL)
            return i;
        stack_push(stack, (int) (node - tree->memoryPool));
    }

    for (i = right + 1; i < end; i++) {
        node = BST_deleteData(tree, points[i] - point);
        if (node == NULL)
            return i;
        stack_push(stack, (int) (node - tree->memoryPool));
    }

    return end;
}

void RT_Insert(const BSTPtr tree, const StackPtr stack, int left, const int right, const int end) {
    int i;

    if (end < left)
        left = end;

    for (i = right + 1; i < end; i++)
        BST_insertNode(tree, tree->memoryPool + stack_pop(stack));

    for (i = 0; i < left; i++)
        BST_insertNode(tree, tree->memoryPool + stack_pop(stack));
}

int reconstructTurnpikeBody(Package *package, const int left, const int right) {
    DistanceType max, isSuccessful = 0;
    int end;

    if (left > right)
        return 1;

    max = BST_findMax(package->tree)->data;

    if (package->npoints ==
        (end = RT_Delete(package->tree, package->stack, package->points, max, left, right, package->npoints))) {
        package->points[right] = max;
        isSuccessful = reconstructTurnpikeBody(package, left, right - 1);
    }

    if (!isSuccessful) {
        RT_Insert(package->tree, package->stack, left, right, end);

        if (package->npoints ==
            (end = RT_Delete(package->tree, package->stack, package->points, max = package->points[package->npoints - 1] - max,
                             left, right, package->npoints))) {
            package->points[left] = max;
            isSuccessful = reconstructTurnpikeBody(package, left + 1, right);
        }

        if (!isSuccessful) {
            RT_Insert(package->tree, package->stack, left, right, end);
        }
    }

    return isSuccessful;
}

void reconstructTurnpike(DistanceType distances[], DistanceType points[], const int npoints) {
    const int numOfDistances = npoints * (npoints - 1) / 2;
    const BSTPtr tree = buildBST(distances, numOfDistances, 1);
    const StackPtr stack = newStack(numOfDistances);
    Package package = (Package){tree, points, stack, npoints};

    points[0] = 0;
    points[npoints - 1] = BST_deleteMax(tree)->data;
    points[npoints - 2] = BST_deleteMax(tree)->data;

    if (BST_deleteData(tree, points[npoints - 1] - points[npoints - 2]) != NULL) {
        if (reconstructTurnpikeBody(&package, 1, npoints - 3)) {
            puts("reconstruct successfully!");
            goto END;
        }
    }

    puts("reconstruct failed!");
END:
    BT_destroy(tree);
    stack_destroy(stack);
}
