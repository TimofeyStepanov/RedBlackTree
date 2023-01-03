#include <stack>
#include <list>

#include "../comparators/ComparatorStrategy.h"

template <typename TKey, typename TData>
class Tree
{
public:
    virtual void add(const TKey& key, const TData& data) = 0;
    virtual void pop(const TKey& key) = 0;
    virtual std::list<TData> find(const TKey& key) const = 0;
    virtual ~Tree() = default;
};

template <typename TKey, typename TData>
class RBTree : public Tree<TKey, TData>
{
private:
    struct KeyAndValue
    {
        TKey key;
        TData data;
    };

    class Node
    {
    public:
        std::list<KeyAndValue> keysAndValues;
        Node *leftPtr, *rightPtr;
        bool isRed;

        Node(const TKey& key, const TData& data)
        {
            KeyAndValue newElement = {key, data};
            keysAndValues.push_back(newElement);

            leftPtr = nullptr;
            rightPtr = nullptr;

            isRed = true;
        }

        void makeRed()
        {
            this->isRed = true;
        }
        void makeBlack()
        {
            this->isRed = false;
        }

        bool nodeIsRed() const
        {
            return isRed == true;
        }
        bool nodeIsBlack() const
        {
            return isRed == false;
        }

        bool hasTheSameKeyWith(Node* node) const
        {
            const TKey& thisKey = keysAndValues.front().key;
            const TKey& nodeKey = node->keysAndValues.front().key;

            return thisKey == nodeKey;
        }

        Node* returnAnotherChild(Node* child) const
        {
            if (leftPtr == child)
                return rightPtr;
            else if (rightPtr == child)
                return leftPtr;
            else if (leftPtr == nullptr)
                return rightPtr;
            else
                return leftPtr;

        }

        bool nodeIsNotLeaf() const
        {
            return !(leftPtr == nullptr && rightPtr == nullptr);
        }

        bool nodeIsBranch() const 
        {
            return (leftPtr && !rightPtr) || (!leftPtr && rightPtr);
        }

        bool nodeIsNotBranch() const 
        {
            return !nodeIsBranch();
        }

        Node* returnRedChildOrNullptr() const
        {
            if (leftPtr != nullptr && leftPtr->nodeIsRed())
                return leftPtr;
            if (rightPtr != nullptr && rightPtr->nodeIsRed())
                return rightPtr;
            return nullptr;
        }

        Node(const Node& node)
        {
            this->keysAndValues = node.keysAndValues;

            this->leftPtr = node.leftPtr;
            this->rightPtr = node.rightPtr;

            this->isRed = node.isRed;
        }

        bool redGrandsonExists() const
        {
            Node* leftChild = leftPtr;
            Node* rightChild = rightPtr;

            Node *leftRedNephew, *rightRedNephew;
            if (leftChild)
                leftRedNephew = leftChild->returnRedChildOrNullptr();
            if (rightChild)
                rightRedNephew = rightChild->returnRedChildOrNullptr();
            
            if (rightRedNephew || leftRedNephew)
                return true;
            return false;
        }

        Node* returnSon(Node* grandson, ComparatorStrategy<TKey>* comparatorStrategy) const 
        {
            const TKey& key = keysAndValues.front().key;
            const TKey& grandsonKey = grandson->keysAndValues.front().key;

            int compareGrandfatherAndGrandson = comparatorStrategy->compare(key, grandsonKey);
            if (compareGrandfatherAndGrandson < 0)
                return rightPtr;
            else
                return leftPtr;
        }

        Node* returnGrandsonByZigzag(Node* greatGrandfather, ComparatorStrategy<TKey>* comparatorStrategy) const
        {
            const TKey& key = this->keysAndValues.front().key;
            const TKey& greatGrandfatherKey = greatGrandfather->keysAndValues.front().key;

            int compareGreatGrandfatherAndGrandfather = comparatorStrategy->compare(greatGrandfatherKey, key);
            Node *grandfather, *father, *grandson;

            if (compareGreatGrandfatherAndGrandfather > 0)
            {
                grandfather = greatGrandfather->leftPtr;
                father = grandfather->rightPtr;
                grandson = father->leftPtr;
            }
            else
            {
                grandfather = greatGrandfather->rightPtr;
                father = grandfather->leftPtr;
                grandson = father->rightPtr;
            }     
            return grandson;
        }

        Node* returnSonByZigzag(Node* grandfather, ComparatorStrategy<TKey>* comparatorStrategy) const
        {
            const TKey& key = keysAndValues.front().key;
            const TKey& grandfatherKey = grandfather->keysAndValues.front().key;

            int compareGrandfatherAndFather =  comparatorStrategy->compare(grandfatherKey, key); 
            if (compareGrandfatherAndFather > 0)
            {
                return rightPtr;
            }
            else
            {
                return leftPtr;
            }
        }

        std::list<TData> returnData() const
        {
            std::list<TData> values;
            for (const KeyAndValue& keyAndValue : keysAndValues)
            {
                values.push_back(keyAndValue.data);
            }
            return values;
        }

        void log(void (*function)(const TKey&, const TData&)) const
        {
            this->nodeIsRed() ? std::cout << "Red " : std::cout << "Black ";

            for (const KeyAndValue& keyAndValue : keysAndValues)
            {
                std::cout << "[";
                function(keyAndValue.key, keyAndValue.data);
                std::cout << "] ";
            }
            std::cout << std::endl;
        }
    };

private:
    Node* head = nullptr;
    ComparatorStrategy<TKey>* comparatorStrategy = nullptr;
    unsigned int numberOfNodes;
    void (*function)(const TKey&, const TData&) = nullptr; // TODO: delete 

public:
    RBTree(ComparatorStrategy<TKey>* comparatorStrategy);

public:
    void add(const TKey& key, const TData& data) override;
private:
    void tryAdd(const TKey& key, const TData& data);
    void initStackOfPreviousNodesInInsert(std::stack<Node*>& nodeStack, const TKey& keyToFind) const;
    void linkOrUnionChildWithFatherInInsert(Node* child, Node* father);

public:
    void pop(const TKey& key) override;
private:
    void tryPop(const TKey& key);
    void initStackOfPreviousNodesInDeletionOrThrowException(std::stack<Node*>& nodeStack, const TKey& keyToFind) const;
    void deleteNode(Node* toDelete, Node* father);
    void deleteBranch(Node* toDelete, Node* father);
    void deleteRedLeaf(Node* toDelete, Node* father);
    void findMaxNodeInLeftBranchAndUpdateStack(std::stack<Node*>& nodeStack) const;
    void deleteLeafOrBranch(std::stack<Node*>& nodeStack);
    void deleteBranchOrRedLeaf(Node* child, Node* father);

public:
    std::list<TData> find(const TKey& key) const override;

public:
    void print(void (*function)(const TKey&, const TData&));
private:
    void doPrint(void (*function)(const TKey&, const TData&), Node* startNode) const;

private:
    void hangNodesAfterTurn(Node* nodeToHang, std::stack<Node*>& nodeStack);
    void hangNodesAfterTurn(Node* nodeToHang, Node* previousNode) const;
    bool needToMakeSingleTurn(Node* grandfather, Node* grandson) const;
    void makeSingleTurn(Node* grandfather, Node* grandson) const;
    void makeDoubleTurn(Node* grandfather, Node* grandson) const;
    void throwExceptionIfThereIsNoCompare() const;
    Node* pullOutNodeFromStack(std::stack<Node*>& nodeStack) const;
    Node* returnFather(Node* grandfather, Node* grandson) const;
    bool isEmpty() const;
    void swapNodes(Node* first, Node* second) const;

public:
    ~RBTree();
private:
    void makeRecursiveRemovalOfNodeForDestructor(Node* ptr) const;
};

template <typename TKey, typename TData>
RBTree<TKey, TData>::RBTree(ComparatorStrategy<TKey>* comparatorStrategy)
{
    head = nullptr;
    numberOfNodes = 0;
    this->comparatorStrategy = comparatorStrategy;
}


template <typename TKey, typename TData>
void RBTree<TKey, TData>::add(const TKey& key, const TData& data)
{
    try
    {
        tryAdd(key, data);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

template <typename TKey, typename TData>
void RBTree<TKey, TData>::tryAdd(const TKey& key, const TData& data)
{
    if (isEmpty())
    {
        head = new Node(key, data);
        head->makeBlack();
        numberOfNodes = 1;
        return;
    }

    throwExceptionIfThereIsNoCompare();

    std::stack<Node*> nodeStack;
    initStackOfPreviousNodesInInsert(nodeStack, key);

    Node* child = new Node(key, data);
    Node* father = pullOutNodeFromStack(nodeStack);

    linkOrUnionChildWithFatherInInsert(child, father);
    if (father->nodeIsBlack() || father->hasTheSameKeyWith(child))
    {
        return;
    }
    
    while (father != nullptr && father->nodeIsRed())
    {
        Node* grandfather = pullOutNodeFromStack(nodeStack);
        Node* uncle = grandfather->returnAnotherChild(father);

        if (uncle == nullptr || uncle->nodeIsBlack()) // makeRotation
        {
            if (needToMakeSingleTurn(grandfather, child))
            {
                makeSingleTurn(grandfather, child);
                hangNodesAfterTurn(father, nodeStack);

                father->makeBlack();
            }
            else
            {
                makeDoubleTurn(grandfather, child);
                hangNodesAfterTurn(child, nodeStack);

                child->makeBlack();
            }
            grandfather->makeRed();
            return;
        }
        else // make repaint
        {
            father->makeBlack();
            uncle->makeBlack();
            grandfather->makeRed();

            child = grandfather;
        }
        father = pullOutNodeFromStack(nodeStack);
    }

    if (head->nodeIsRed())
        head->makeBlack();
}

template <typename TKey, typename TData>
void RBTree<TKey, TData>::initStackOfPreviousNodesInInsert(
        std::stack<Node*>& nodeStack,
        const TKey& keyToFind) const {
            
    Node* nodePtr = head;
    while (nodePtr)
    {
        nodeStack.push(nodePtr);
        
        const TKey& key = nodePtr->keysAndValues.front().key;
        int compareResult = comparatorStrategy->compare(key, keyToFind);

        if (compareResult < 0)
        {
            nodePtr = nodePtr->rightPtr;
        }
        else if (compareResult > 0)
        {
            nodePtr = nodePtr->leftPtr;
        }
        else
        {
            return;
        }
    }
}

template <typename TKey, typename TData>
void RBTree<TKey, TData>::linkOrUnionChildWithFatherInInsert(Node* child, Node* father)
{
    const TKey& fatherKey = father->keysAndValues.front().key;
    const TKey& childKey = child->keysAndValues.front().key;

    int compareFatherAndChild = comparatorStrategy->compare(fatherKey, childKey);
    if (compareFatherAndChild < 0)
    {
        father->rightPtr = child;
        numberOfNodes++;
    }
    else if (compareFatherAndChild > 0)
    {
        father->leftPtr = child;
        numberOfNodes++;
    }
    else
    {
        KeyAndValue newElement = {child->keysAndValues.front().key, child->keysAndValues.front().data};
        father->keysAndValues.push_back(newElement);
    }
}


template <typename TKey, typename TData>
void RBTree<TKey,TData>::pop(const TKey& key)
{
    try
    {
        tryPop(key);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::tryPop(const TKey& key)
{
    if (isEmpty())
    {
        throw std::invalid_argument("Can't do pop. Tree is empty!");
    }

    throwExceptionIfThereIsNoCompare();

    std::stack<Node*> nodeStack;
    initStackOfPreviousNodesInDeletionOrThrowException(nodeStack, key);

    Node* child = nodeStack.top();
    if (child->nodeIsNotLeaf() && child->nodeIsNotBranch())
    {
        findMaxNodeInLeftBranchAndUpdateStack(nodeStack);
    }
    deleteLeafOrBranch(nodeStack);
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::initStackOfPreviousNodesInDeletionOrThrowException(
        std::stack<Node*> &nodeStack, 
        const TKey& keyToFind) const {

    Node* nodePtr = head;
    while (nodePtr)
    {
        nodeStack.push(nodePtr);
        const TKey& nodePtrKey = nodePtr->keysAndValues.front().key;
        int compareResult = comparatorStrategy->compare(nodePtrKey, keyToFind);

        if (compareResult < 0)
        {
            nodePtr = nodePtr->rightPtr;
        }
        else if (compareResult > 0)
        {
            nodePtr = nodePtr->leftPtr;
        }
        else
        {
            return;
        }
    }
    throw std::invalid_argument("No element in tree!");
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::findMaxNodeInLeftBranchAndUpdateStack(std::stack<Node*>& nodeStack) const
{
    Node* child = nodeStack.top();
    Node* minimalNode = child->leftPtr;
    nodeStack.push(minimalNode);

    while (minimalNode->rightPtr)
    {
        minimalNode = minimalNode->rightPtr;
        nodeStack.push(minimalNode);
    }

    swapNodes(child, minimalNode);
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::deleteLeafOrBranch(std::stack<Node*>& nodeStack)
{
    Node* childToDelete = pullOutNodeFromStack(nodeStack);
    Node child = *childToDelete;
    Node* childPtr = &child;

    Node* father = pullOutNodeFromStack(nodeStack);

    if (numberOfNodes == 1)
    {
        delete head;
        numberOfNodes = 0;
        head = nullptr;
        return;
    }

    if (child.nodeIsRed() || child.nodeIsBranch())
    {
        deleteBranchOrRedLeaf(childToDelete, father);
        return;
    }

    // child is black
    deleteNode(childToDelete, father);
    while(father)
    {
        Node* brother = father->returnAnotherChild(childPtr);
        Node* redNephew = brother->returnRedChildOrNullptr();

        if (father->nodeIsRed())
        {
            if (redNephew)
            {
                if (needToMakeSingleTurn(father, redNephew))
                {
                    makeSingleTurn(father, redNephew);
                    hangNodesAfterTurn(brother, nodeStack);

                    brother->makeRed();
                    father->makeBlack();
                    redNephew->makeBlack();
                }
                else
                {
                    makeDoubleTurn(father, redNephew);
                    hangNodesAfterTurn(redNephew, nodeStack);

                    father->makeBlack();
                }
            } 
            else
            {
                father->makeBlack();
                brother->makeRed();
            }
            return;
        }
        else // father is black
        {
            if (brother->nodeIsRed())
            {
                Node *blackNephew, *anotherBlackNephew;

                if (brother->redGrandsonExists())
                {
                    Node* brotherGrandson = brother->returnGrandsonByZigzag(father, comparatorStrategy);
                    if (brotherGrandson && brotherGrandson->nodeIsRed())
                    {
                        blackNephew = brother->returnSon(brotherGrandson, comparatorStrategy);
                
                        makeDoubleTurn(father, blackNephew);
                        hangNodesAfterTurn(blackNephew, nodeStack);

                        brotherGrandson->makeBlack();
                        return;
                    }
                    
                    blackNephew = brother->returnSonByZigzag(father, comparatorStrategy);
                    brotherGrandson = blackNephew->returnAnotherChild(brotherGrandson);
                    if (brotherGrandson && brotherGrandson->nodeIsRed())
                    {
                        makeSingleTurn(father, brother->returnAnotherChild(blackNephew));
                        hangNodesAfterTurn(brother, nodeStack);
                        brother->makeBlack();

                        makeDoubleTurn(father, brotherGrandson);
                        hangNodesAfterTurn(brotherGrandson, brother);
                        return;
                    }
                    
                    blackNephew->makeRed();
                    brother->makeBlack();

                    blackNephew = brother->returnAnotherChild(blackNephew);
                    makeSingleTurn(father, blackNephew);
                    hangNodesAfterTurn(brother, nodeStack);
                    return;
                }
                else
                {
                    blackNephew = brother->leftPtr;
                    anotherBlackNephew = brother->rightPtr;

                    if (needToMakeSingleTurn(father, blackNephew))
                    {
                        makeSingleTurn(father, blackNephew);
                        hangNodesAfterTurn(brother, nodeStack);
                    }
                    else
                    {
                        makeDoubleTurn(father, blackNephew);
                        hangNodesAfterTurn(blackNephew, nodeStack);
                    }

                    brother->makeBlack();
                    anotherBlackNephew->makeRed();
                }
                return;
            }
            else // brother is black
            {
                if (redNephew)
                {
                    if (needToMakeSingleTurn(father, redNephew))
                    {
                        makeSingleTurn(father, redNephew);
                        hangNodesAfterTurn(brother, nodeStack);
                    }
                    else
                    {
                        makeDoubleTurn(father, redNephew);
                        hangNodesAfterTurn(redNephew, nodeStack);
                    }
                    redNephew->makeBlack();
                    return;
                }
                else
                {
                    brother->makeRed();
                    childPtr = father;
                    father = pullOutNodeFromStack(nodeStack);
                }
            }
        }
    }
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::deleteNode(Node* toDelete, Node* father)
{
    if (father->leftPtr == toDelete)
        father->leftPtr = nullptr;
    else
        father->rightPtr = nullptr;

    delete toDelete;
    numberOfNodes--;
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::deleteBranchOrRedLeaf(Node* child, Node* father)
{
    if (child->nodeIsBranch())
    {
        deleteBranch(child, father);
    }
    else
    {
        deleteRedLeaf(child, father);
    }
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>:: deleteBranch(Node* toDelete, Node* father)
{
    Node* toHang;
    if (toDelete->leftPtr)
    {
        toHang = toDelete->leftPtr; 
    }
    else
    {
        toHang = toDelete->rightPtr;
    }

    if (father)
    {
        const TKey& fatherKey = father->keysAndValues.front().key;
        const TKey& toHangKey = toHang->keysAndValues.front().key;
        comparatorStrategy->compare(fatherKey, toHangKey) < 0 ? father->rightPtr = toHang : father->leftPtr = toHang;
    }
    else {
        head = toHang;
    }
    toHang->makeBlack();

    numberOfNodes--;
    delete toDelete;
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::deleteRedLeaf(Node* toDelete, Node* father)
{
    if (father->rightPtr == toDelete)
    {
        father->rightPtr = nullptr;
    }
    else
    {
        father->leftPtr = nullptr;
    }
    numberOfNodes--;
    delete toDelete;
}


template <typename TKey, typename TData>
std::list<TData> RBTree<TKey, TData>::find(const TKey& key) const
{
    Node* ptr = head;
    while (ptr)
    {
        const TKey& ptrKey = ptr->keysAndValues.front().key;
        int compareCurrentAndNeededKeys = comparatorStrategy->compare(ptrKey, key);
        if (compareCurrentAndNeededKeys < 0)
        {
            ptr = ptr->rightPtr;
        }
        else if (compareCurrentAndNeededKeys > 0)
        {
            ptr = ptr->leftPtr;
        }
        else
        {
            return ptr->returnData();
        }
    }
    throw std::invalid_argument("No element in tree!");
}


template <typename TKey, typename TData>
void RBTree<TKey, TData>::print(void (*function)(const TKey&, const TData&))
{
    if (!function)
    {
        std::cout << "No function" << std::endl;
        return;
    }

    if (head)
    {
        std::cout << "Size " << numberOfNodes << std::endl; 
        doPrint(function, head);
    }
    else
        std::cout << "Tree is empty!" << std::endl;
    // this->function = function; // to save
}

template <typename TKey, typename TData>
void RBTree<TKey, TData>::doPrint(void (*function)(const TKey&, const TData&), Node* startNode) const
{
    startNode->log(function);

    if (startNode->leftPtr)
    {
        doPrint(function, startNode->leftPtr);
    }
    if (startNode->rightPtr)
    {
        doPrint(function, startNode->rightPtr);
    }
}


template <typename TKey, typename TData>
void RBTree<TKey, TData>::hangNodesAfterTurn(Node* nodeToHang, std::stack<Node*>& nodeStack)
{
    if (nodeStack.empty())
    {
        head = nodeToHang;
        return;
    }

    Node* previousNode = nodeStack.top();

    const TKey& previousNodeKey = previousNode->keysAndValues.front().key;
    const TKey& nodeToHangKey = nodeToHang->keysAndValues.back().key;

    int compareNodes = comparatorStrategy->compare(previousNodeKey, nodeToHangKey);
    compareNodes < 0 ? previousNode->rightPtr = nodeToHang : previousNode->leftPtr = nodeToHang;
}

template <typename TKey, typename TData>
void RBTree<TKey, TData>::hangNodesAfterTurn(Node* nodeToHang, Node* previousNode) const
{
    const TKey& previousNodeKey = previousNode->keysAndValues.front().key;
    const TKey& nodeToHangKey = nodeToHang->keysAndValues.back().key;

    int compareNodes = comparatorStrategy->compare(previousNodeKey, nodeToHangKey);
    compareNodes < 0 ? previousNode->rightPtr = nodeToHang : previousNode->leftPtr = nodeToHang;
}

template <typename TKey, typename TData>
bool RBTree<TKey, TData>::needToMakeSingleTurn(Node*grandfather, Node* grandson) const
{
    Node* father = returnFather(grandfather, grandson);

    const TKey& grandfatherKey = grandfather->keysAndValues.front().key;
    const TKey& fatherKey = father->keysAndValues.front().key;
    const TKey& grandsonKey = grandson->keysAndValues.front().key;

    int compareGrandfatherAndFather = comparatorStrategy->compare(grandfatherKey, fatherKey);
    int compareFatherAndGrandson = comparatorStrategy->compare(fatherKey, grandsonKey);

    return compareGrandfatherAndFather == compareFatherAndGrandson;
}

template <typename TKey, typename TData>
void RBTree<TKey, TData>::makeSingleTurn(Node* grandfather, Node* grandson) const
{
    Node* father = returnFather(grandfather, grandson);

    if (father->rightPtr == grandson)
    {
        grandfather->rightPtr = father->leftPtr;
        father->leftPtr = grandfather;
    }
    else
    {
        grandfather->leftPtr = father->rightPtr;
        father->rightPtr = grandfather;
    }
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::makeDoubleTurn(Node* grandfather, Node* grandson) const
{
    Node *father = returnFather(grandfather, grandson);

    if (father->leftPtr == grandson)
    {
        father->leftPtr = grandson->rightPtr;
        grandfather->rightPtr = grandson->leftPtr;

        grandson->rightPtr = father;
        grandson->leftPtr = grandfather;
    }
    else
    {
        father->rightPtr = grandson->leftPtr;
        grandfather->leftPtr = grandson->rightPtr;

        grandson->leftPtr = father;
        grandson->rightPtr = grandfather;
    }
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::throwExceptionIfThereIsNoCompare() const
{
    if (comparatorStrategy == nullptr)
        throw std::overflow_error("Can't use Compare!");
}

template <typename TKey, typename TData>
typename RBTree<TKey, TData>::Node* RBTree<TKey, TData>::pullOutNodeFromStack(std::stack<Node*>& nodeStack) const
{
    if (nodeStack.size() == 0)
        return nullptr;

    Node *nodeToReturn = nodeStack.top();
    nodeStack.pop();
    return nodeToReturn;
}

template <typename TKey, typename TData>
typename RBTree<TKey,TData>::Node* RBTree<TKey, TData>::returnFather(Node* grandfather, Node* grandson) const
{
    const TKey& grandfatherKey = grandfather->keysAndValues.front().key;
    const TKey& grandsonKey = grandson->keysAndValues.front().key;
    int compareGrandfatherAndGrandson = comparatorStrategy->compare(grandfatherKey, grandsonKey);

    Node *father;
    compareGrandfatherAndGrandson < 0 ? father = grandfather->rightPtr : father = grandfather->leftPtr;
    return father;
}

template <typename TKey, typename TData>
bool RBTree<TKey,TData>::isEmpty() const
{
    return numberOfNodes == 0;
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::swapNodes(Node* first, Node* second) const
{
    const std::list<KeyAndValue>& firstKeysAndValues = first->keysAndValues;
    
    first->keysAndValues = second->keysAndValues;
    second->keysAndValues = firstKeysAndValues;
}

template <typename TKey, typename TData>
RBTree<TKey,TData>::~RBTree()
{
    
    // while (head)
    // {
    //     const TKey& key = head->keysAndValues.front().key;
    //     this->pop(key);
    // }

    if (head)
        makeRecursiveRemovalOfNodeForDestructor(head);

    std::cout << "Success" << std::endl;
}

template <typename TKey, typename TData>
void RBTree<TKey,TData>::makeRecursiveRemovalOfNodeForDestructor(Node* ptr) const
{
    if (ptr->leftPtr)
    {
        makeRecursiveRemovalOfNodeForDestructor(ptr->leftPtr);
    }

    if (ptr->rightPtr)
    {
        makeRecursiveRemovalOfNodeForDestructor(ptr->rightPtr);
    }

    delete ptr;
}

