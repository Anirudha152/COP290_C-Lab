#include "set.h"
#include <stdlib.h>

static int compare_cells(const int cell_index_1, const int cell_index_2) {
    return cell_index_1 - cell_index_2;
}

static int height(SetNode *node) {
    return node ? node->height : 0;
}

static int balance_factor(SetNode *node) {
    return node ? height(node->left) - height(node->right) : 0;
}

static void update_height(SetNode *node) {
    const int left_height = height(node->left);
    const int right_height = height(node->right);
    node->height = (left_height > right_height ? left_height : right_height) + 1;
}

static void update_parent(SetNode *node, SetNode *new_parent) {
    if (node) {
        node->parent = new_parent;
    }
}

static SetNode* rotate_right(Set *set, SetNode *y) {
    SetNode *x = y->left;
    SetNode *T2 = x->right;

    x->right = y;
    y->left = T2;
    x->parent = y->parent;
    y->parent = x;
    update_parent(T2, y);
    if (y == set->root) {
        set->root = x;
    } else {
        if (x->parent->left == y) {
            x->parent->left = x;
        } else {
            x->parent->right = x;
        }
    }

    update_height(y);
    update_height(x);

    return x;
}

static SetNode* rotate_left(Set *set, SetNode *x) {
    SetNode *y = x->right;
    SetNode *T2 = y->left;

    y->left = x;
    x->right = T2;
    y->parent = x->parent;
    x->parent = y;
    update_parent(T2, x);
    if (x == set->root) {
        set->root = y;
    } else {
        if (y->parent->left == x) {
            y->parent->left = y;
        } else {
            y->parent->right = y;
        }
    }

    update_height(x);
    update_height(y);

    return y;
}

static SetNode* create_node(int cell_index) {
    SetNode *node = malloc(sizeof(SetNode));
    if (!node) return NULL;
    node->cell_index = cell_index;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->height = 1;
    return node;
}

static void balance_up_to_root(Set *set, SetNode *node) {
    SetNode *current = node;
    while (current) {
        update_height(current);
        const int balance = balance_factor(current);
        if (balance > 1) {
            if (balance_factor(current->left) < 0) {
                current->left = rotate_left(set, current->left);
            }
            current = rotate_right(set, current);
        }
        else if (balance < -1) {
            if (balance_factor(current->right) > 0) {
                current->right = rotate_right(set, current->right);
            }
            current = rotate_left(set, current);
        }
        current = current->parent;
    }
}

static SetNode* find_node(SetNode *root, const int cell_index) {
    SetNode *current = root;
    while (current) {
        const int cmp = compare_cells(current->cell_index, cell_index);
        if (cmp == 0) return current;
        if (cmp > 0) current = current->left;
        else current = current->right;
    }
    return NULL;
}

static SetNode* find_min(SetNode *node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

static SetNode* find_successor(const SetNode *node) {
    if (node->right) {
        return find_min(node->right);
    }
    SetNode *parent = node->parent;
    while (parent && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

Set* set_create(void) {
    Set *set = malloc(sizeof(Set));
    if (set) {
        set->root = NULL;
        set->size = 0;
    }
    return set;
}

void set_destroy(Set *set) {
    if (!set) return;
    SetNode *current = set->root;
    while (current) {
        if (current->left) {
            SetNode *left = current->left;
            current->left = NULL;
            current = left;
            continue;
        }
        if (current->right) {
            SetNode *right = current->right;
            current->right = NULL;
            current = right;
            continue;
        }
        SetNode *parent = current->parent;
        free(current);
        current = parent;
    }
    free(set);
}

int set_insert(Set *set, int cell_index) {
    if (!set) return 0;
    if (!set->root) {
        set->root = create_node(cell_index);
        if (!set->root) return 0;
        set->size++;
        return 1;
    }
    SetNode *current = set->root;
    SetNode *parent = NULL;
    while (current) {
        parent = current;
        const int cmp = compare_cells(cell_index, current->cell_index);
        if (cmp == 0) return 0;
        if (cmp < 0) current = current->left;
        else current = current->right;
    }
    SetNode *new_node = create_node(cell_index);
    if (!new_node) return 0;
    new_node->parent = parent;
    if (compare_cells(cell_index, parent->cell_index) < 0) {
        parent->left = new_node;
    } else {
        parent->right = new_node;
    }
    set->size++;
    balance_up_to_root(set, new_node);
    return 1;
}

int set_remove(Set *set, const int cell_index) {
    if (!set) return 0;
    SetNode *node = find_node(set->root, cell_index);
    if (!node) return 0;
    SetNode *balance_start = NULL;
    if (!node->left || !node->right) {
        SetNode *child = node->left ? node->left : node->right;
        SetNode *parent = node->parent;
        if (child) {
            child->parent = parent;
        }
        if (!parent) {
            set->root = child;
        } else {
            if (parent->left == node) {
                parent->left = child;
            } else {
                parent->right = child;
            }
            balance_start = parent;
        }

        free(node);
    }
    else {
        SetNode *successor = find_successor(node);
        node->cell_index = successor->cell_index;

        SetNode *parent = successor->parent;
        SetNode *child = successor->right;

        if (parent->left == successor) {
            parent->left = child;
        } else {
            parent->right = child;
        }

        if (child) {
            child->parent = parent;
        }

        balance_start = parent;
        free(successor);
    }

    set->size--;
    if (balance_start) {
        balance_up_to_root(set, balance_start);
    }
    return 1;
}

int set_find(Set *set, const int cell_index) {
    if (!set) return 0;
    const SetNode *node = find_node(set->root, cell_index);
    return node ? 1 : 0;
}

void set_clear(Set *set) {
    if (!set) return;
    SetNode *current = set->root;
    while (current) {
        if (current->left) {
            SetNode *left = current->left;
            current->left = NULL;
            current = left;
            continue;
        }
        if (current->right) {
            SetNode *right = current->right;
            current->right = NULL;
            current = right;
            continue;
        }
        SetNode *parent = current->parent;
        free(current);
        current = parent;
    }
    set->root = NULL;
    set->size = 0;
}

size_t set_size(const Set *set) {
    return set ? set->size : 0;
}

SetIterator* set_iterator_create(Set *set) {
    if (!set) return NULL;

    SetIterator *iterator = malloc(sizeof(SetIterator));
    if (!iterator) return NULL;

    // Find the leftmost node
    SetNode *current = set->root;
    while (current && current->left) {
        current = current->left;
    }

    iterator->current = current;
    return iterator;
}

int set_iterator_next(SetIterator *iterator) {
    if (!iterator || !iterator->current) return -1;

    const int cell_index = iterator->current->cell_index;
    iterator->current = find_successor(iterator->current);
    return cell_index;
}

void set_iterator_destroy(SetIterator *iterator) {
    free(iterator);
}