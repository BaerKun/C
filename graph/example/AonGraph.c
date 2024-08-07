#include "AonGraph.h"
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

static void InitIndegree(AonGraphPtr pAonGraph, int inDegree[]) {
    NodeId nodeId;

    for (nodeId = 0; nodeId < pAonGraph->nodeNum; nodeId++)
        inDegree[nodeId] = pAonGraph->nodes[nodeId].inDegree;
}

static void _buildCriticalPath(AonGraphPtr pAonGraph, QueuePtr queue, int inDegree[]) {
    NodeId nodeId, successorId;
    AdjacencyListPtr pAdjacencyList;
    ActivityNodePtr nodes, pThisNode, pSuccessor;
    nodes = pAonGraph->nodes;

    while (queue->front != queue->rear) {
        nodeId = dequeue(queue);
        pThisNode = nodes + nodeId;
        for (pAdjacencyList = pThisNode->pSuccessorList; pAdjacencyList; pAdjacencyList = pAdjacencyList->next) {
            successorId = pAdjacencyList->id;
            pSuccessor = nodes + successorId;
            if (pSuccessor->earlyStart < pThisNode->earlyStart + pThisNode->duration)
                pSuccessor->earlyStart = pThisNode->earlyStart + pThisNode->duration;
            if (--inDegree[successorId] == 0)
                enqueue(queue, successorId);
        }
    }

    pThisNode->lateStart = pThisNode->earlyStart;
    pThisNode->slack = 0;

    NodeId *topSort = queue->elements;

    for(int i = pAonGraph->nodeNum - 1; i >= 0; i--) {
        nodeId = topSort[i];
        pThisNode = nodes + nodeId;
        for (pAdjacencyList = pThisNode->pSuccessorList; pAdjacencyList; pAdjacencyList = pAdjacencyList->next) {
            successorId = pAdjacencyList->id;
            pSuccessor = nodes + successorId;
            if (pThisNode->lateStart > pSuccessor->lateStart - pThisNode->duration) {
                pThisNode->lateStart = pSuccessor->lateStart - pThisNode->duration;
                if (pThisNode->earlyStart == pThisNode->lateStart &&
                        pSuccessor->slack == 0)
                    pThisNode->path = successorId;
            }
        }
        pThisNode->slack = pThisNode->lateStart - pThisNode->earlyStart;
    }
}

AonGraphPtr CreateAonGraph(int nodeNum, const int *activityDurations) {
    AonGraphPtr pAonGraph = (AonGraphPtr) malloc(sizeof(AonGraph));
    for (pAonGraph->capacity = INITIAL_NODES_NUMBER; pAonGraph->capacity < nodeNum; pAonGraph->capacity *= 2);
    ActivityNodePtr nodes = (ActivityNodePtr) malloc(sizeof(ActivityNode) * pAonGraph->capacity);
    pAonGraph->nodes = nodes;
    pAonGraph->nodeNum = nodeNum;
    for (NodeId nodeId = 0; nodeId < nodeNum; nodeId++) {
        nodes[nodeId].duration = activityDurations[nodeId];
        nodes[nodeId].earlyStart = 0;
        nodes[nodeId].lateStart = INFINITY;
        nodes[nodeId].inDegree = 0;
        nodes[nodeId].path = INFINITY;
        nodes[nodeId].pSuccessorList = NULL;
    }

    return pAonGraph;
}

void DeleteAonGraph(AonGraphPtr pAonGraph) {
    AdjacencyListPtr pThisNode, pNextNode;

    for (NodeId nodeId = 0; nodeId < pAonGraph->nodeNum; nodeId++)
        for (pThisNode = pAonGraph->nodes[nodeId].pSuccessorList; pThisNode; pThisNode = pNextNode) {
            pNextNode = pThisNode->next;
            free(pThisNode);
        }

    free(pAonGraph);
}

void AddActivityNode(AonGraphPtr pAonGraph, TimeType duration) {
    if (pAonGraph->nodeNum == pAonGraph->capacity) {
        pAonGraph->capacity *= 2;
        if (realloc(pAonGraph->nodes, sizeof(ActivityNode) * pAonGraph->capacity) == NULL) {
            fputs("AddActivityNode: realloc failed\n", stderr);
            return;
        }
    }

    pAonGraph->nodes[pAonGraph->nodeNum++].duration = duration;
}

void EstablishDependency(AonGraphPtr pAonGraph, NodeId start, NodeId end) {
    if (start < 0 || start >= pAonGraph->nodeNum || end < 0 || end >= pAonGraph->nodeNum) {
        fputs("EstablishDependency: invalid node id\n", stderr);
        return;
    }
    AdjacencyListPtr pSuccessorList;
    ActivityNodePtr pPredecessorNode = pAonGraph->nodes + start;
    for (pSuccessorList = pPredecessorNode->pSuccessorList;
         pSuccessorList && pSuccessorList->id != end; pSuccessorList = pSuccessorList->next);
    if (pSuccessorList)
        return;
    pSuccessorList = (AdjacencyListPtr) malloc(sizeof(struct Edge));
    pSuccessorList->id = end;
    pSuccessorList->next = pPredecessorNode->pSuccessorList;
    pPredecessorNode->pSuccessorList = pSuccessorList;
    pAonGraph->nodes[end].inDegree++;
}

void BuildCriticalPath(AonGraphPtr pAonGraph) {
    int nodeNum = pAonGraph->nodeNum;
    QueuePtr queue;
    int inDegree[nodeNum];

    queue = createQueue(nodeNum);
    InitIndegree(pAonGraph, inDegree);
    for (NodeId nodeId = 0; nodeId < nodeNum; nodeId++) {
        if (!inDegree[nodeId])
            enqueue(queue, nodeId);
    }
    if (queue->rear == 0) {
        fputs("BuildCriticalPath:HasCycle\n", stderr);
        return;
    }

    _buildCriticalPath(pAonGraph, queue, inDegree);
}

void CopyPath(AonGraphPtr pAonGraph, NodeId copyArray[]) {
    int counter;
    NodeId nodeId;

    for (nodeId = 0; nodeId < pAonGraph->nodeNum; nodeId++) {
        if (pAonGraph->nodes[nodeId].inDegree == 0)
            break;
    }
    for(counter = 0; nodeId != INFINITY; nodeId = pAonGraph->nodes[nodeId].path)
        copyArray[counter++] = nodeId;
}