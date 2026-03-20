#include "NL/nlAVLTree.h"

#include "types.h"

/**
 * Offset/Address/Size: 0x0 | 0x801CE120 | size: 0x490
 */
AVLTreeNode* AVLTreeUntemplated::RemoveAVLNode(AVLTreeNode** root, void* key, unsigned int height)
{
    return nullptr;
}

/**
 * Offset/Address/Size: 0x490 | 0x801CE5B0 | size: 0x398
 */
u32 AVLTreeUntemplated::AddAVLNode(AVLTreeNode** rootNode, void* key, void* value, AVLTreeNode** existingNode, unsigned int height)
{
    AVLTreeNode** root;
    AVLTreeNode** existing;
    AVLTreeUntemplated* self;
    void* searchKey;
    void* entryValue;
    AVLTreeNode* balance;
    AVLTreeNode* father;
    AVLTreeNode* next;
    AVLTreeNode* underBalance;
    AVLTreeNode* currSearch;
    AVLTreeNode* currBalance;
    AVLTreeNode* currAdjust;
    AVLTreeNode* newNode;
    unsigned int lessThanBalance;
    int comp;
    signed char* pathInfo;
    unsigned int stackTop;
    unsigned int balanceSpot;

    root = rootNode;
    existing = existingNode;
    self = this;
    searchKey = key;
    entryValue = value;

    *existing = NULL;

    next = *root;
    if (next == NULL)
    {
        *root = self->AllocateEntry(searchKey, entryValue);
        return 1;
    }

    balance = next;
    currSearch = next;
    father = NULL;

    pathInfo = (signed char*)__alloca(height + 1);

    stackTop = 0;
    balanceSpot = 0;

    while (true)
    {
        comp = self->CompareKey(searchKey, currSearch);
        pathInfo[stackTop] = comp;
        stackTop++;

        if (comp == 0)
        {
            *existing = currSearch;
            return 0;
        }

        if (comp > 0)
        {
            next = currSearch->right;
            if (next == NULL)
            {
                newNode = self->AllocateEntry(searchKey, entryValue);
                currSearch->right = newNode;
                break;
            }
        }
        else
        {
            next = currSearch->left;
            if (next == NULL)
            {
                newNode = self->AllocateEntry(searchKey, entryValue);
                currSearch->left = newNode;
                break;
            }
        }

        if (next->heavy != 0)
        {
            father = currSearch;
            balance = next;
            balanceSpot = stackTop;
        }

        currSearch = next;
    }

    comp = pathInfo[balanceSpot];
    lessThanBalance = ((unsigned int)comp) >> 31;

    if (lessThanBalance)
    {
        underBalance = balance->left;
    }
    else
    {
        underBalance = balance->right;
    }

    currAdjust = underBalance;
    while (currAdjust != newNode)
    {
        signed char path = pathInfo[++balanceSpot];

        if (path < 0)
        {
            currAdjust->heavy = -1;
            currAdjust = currAdjust->left;
        }
        else
        {
            currAdjust->heavy = 1;
            currAdjust = currAdjust->right;
        }
    }

    if (lessThanBalance)
    {
        if (balance->heavy == 0)
        {
            balance->heavy = -1;
            return 1;
        }

        if (balance->heavy < 0)
        {
            if (underBalance->heavy < 0)
            {
                balance->left = underBalance->right;
                underBalance->right = balance;
                balance->heavy = 0;
                underBalance->heavy = 0;
                currBalance = underBalance;
            }
            else
            {
                currBalance = underBalance->right;
                underBalance->right = currBalance->left;
                currBalance->left = underBalance;
                balance->left = currBalance->right;
                currBalance->right = balance;

                if (currBalance->heavy == 0)
                {
                    balance->heavy = 0;
                    underBalance->heavy = 0;
                }
                else if (currBalance->heavy < 0)
                {
                    balance->heavy = 1;
                    underBalance->heavy = 0;
                }
                else
                {
                    balance->heavy = 0;
                    underBalance->heavy = -1;
                }

                currBalance->heavy = 0;
            }
        }
        else
        {
            balance->heavy = 0;
            return 0;
        }
    }
    else
    {
        if (balance->heavy == 0)
        {
            balance->heavy = 1;
            return 1;
        }

        if (balance->heavy > 0)
        {
            if (underBalance->heavy > 0)
            {
                balance->right = underBalance->left;
                underBalance->left = balance;
                balance->heavy = 0;
                underBalance->heavy = 0;
                currBalance = underBalance;
            }
            else
            {
                currBalance = underBalance->left;
                underBalance->left = currBalance->right;
                currBalance->right = underBalance;
                balance->right = currBalance->left;
                currBalance->left = balance;

                if (currBalance->heavy == 0)
                {
                    balance->heavy = 0;
                    underBalance->heavy = 0;
                }
                else if (currBalance->heavy > 0)
                {
                    balance->heavy = -1;
                    underBalance->heavy = 0;
                }
                else
                {
                    balance->heavy = 0;
                    underBalance->heavy = 1;
                }

                currBalance->heavy = 0;
            }
        }
        else
        {
            balance->heavy = 0;
            return 0;
        }
    }

    if (father == NULL)
    {
        *root = currBalance;
    }
    else if (balance == father->right)
    {
        father->right = currBalance;
    }
    else
    {
        father->left = currBalance;
    }

    return 0;
}
