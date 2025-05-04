#include "hw2.h"
#include "monitor.h"

// Store encapsulation based on monitor.h
// Tracking and management of shared inventory of items with customer/supplier maintenance.
class Store : public Monitor {
private:
    // To protect from direct access/altering.
    int cap_limit[3];        // Capacity given for AAA, BBB, CCC normally given as cap in problem
    int inventory[3];      // Current inventory stock of each item type given as avail
    int pS[3];   // Pending supply reserved to suppliers == pS
    int mOpi;      // Max ordering for a customer per item.
    
    Condition cW;     // Customer Wait == cW Condition
    Condition* sW[3]; // 3 suppliers for three items, each a thread with a cond't to a item for supplier to wait.SEach supplier thread One condition variable per item type for supplier wait == sW

public:
    // Store Constructor
    Store() : cW(this) {
        sW[0] = new Condition(this);
        sW[1] = new Condition(this);
        sW[2] = new Condition(this);
    }

    // cap_limit and inventory stock initialization with proper synchronization
    void init(int cA, int cB, int cC, int maxOrder) {
        __synchronized__;
        cap_limit[AAA] = inventory[AAA] = cA; // as defined in question
        cap_limit[BBB] = inventory[BBB] = cB;
        cap_limit[CCC] = inventory[CCC] = cC;
        pS[AAA] = pS[BBB] = pS[CCC] = 0;
        mOpi = maxOrder;
    }

    // Item purchase by customer buy call of (aA, aB, aC) items.
    void buy(int aA, int aB, int aC) {
        __synchronized__;
        // Wait to have availability for requiested items at the same time, cannot buy without having all available as given in problem.
        while (aA > inventory[AAA] || aB > inventory[BBB] || aC > inventory[CCC]) {
            cW.wait();
        }
        // Stock reduction of the purchased amount from items.
        inventory[AAA] -= aA;
        inventory[BBB] -= aB;
        inventory[CCC] -= aC;

        // Supplier informing for open capacity.
        if (aA > 0) sW[AAA]->notifyAll();
        if (aB > 0) sW[BBB]->notifyAll();
        if (aC > 0) sW[CCC]->notifyAll();
    }

    // Supplier calls for space for reservation to supply an item type("AAA", "BBB", "CCC")
    void maysupply(int item, int n) {
        __synchronized__;
        // Space check and wait for space.
        while (inventory[item] + pS[item] + n > cap_limit[item]) {
            sW[item]->wait();
        }
        pS[item] += n; // Pending space now reserved
    }

    // Supplier supplies after conditions available, and commits to supply
    void supply(int item, int n) {
        __synchronized__;
        inventory[item] += n;       // Stock increase after supply.
        pS[item] -= n;    // Drop reservation count.
        cW.notifyAll();      // Supply notification to customer
        sW[item]->notifyAll(); // Supply notification to supplier
    }

    // Monitors capacities and stocks
    void monitor(int cap[3], int inv[3]) {
        __synchronized__;
        for (int i = 0; i < 3; ++i) {
            cap[i] = cap_limit[i];
            inv[i] = inventory[i];
        }
    }
};

static Store store;

// Interface calls.
void initStore(int cA, int cB, int cC, int maxOrder) {
    store.init(cA, cB, cC, maxOrder);
}

void buy(int aA, int aB, int aC) {
    store.buy(aA, aB, aC);
}

void maysupply(int itemtype, int n) {
    store.maysupply(itemtype, n);
}

void supply(int itemtype, int n) {
    store.supply(itemtype, n);
}

void monitorStore(int cap[3], int inv[3]) {
    store.monitor(cap, inv);
}
