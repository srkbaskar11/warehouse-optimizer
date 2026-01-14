#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
using namespace std;
// Zone types
enum ZoneType {
 HOT_ZONE, // High frequency items (close to entry)
 COLD_ZONE // Low frequency items (farther from entry)
};
// Structure to represent a shelf location
struct Shelf {
 int x, y;
 string coordinateName;
 ZoneType zone;
 int maxCapacity;
 int currentLoad;
 string itemStored; // Single item per shelf
 
 Shelf(int x = 0, int y = 0, string coordName = "", ZoneType z = COLD_ZONE) 
 : x(x), y(y), coordinateName(coordName), zone(z), maxCapacity(1000), currentLoad(0), 
itemStored("") {}
 
 bool isEmpty() const {
 return itemStored.empty();
 }
 
 bool canStore(int itemSize) {
 return isEmpty() && (itemSize <= maxCapacity);
 }
 
 void addItem(string itemName, int size) {
 itemStored = itemName;
 currentLoad = size;
 }
 
 void removeItem() {
 itemStored = "";
 currentLoad = 0;
 }
 
 double distanceToEntry() {
 return sqrt(pow(x, 2) + pow(y, 2));
 }
};
// Structure for an item with frequency
struct Item {
 string name;
 int frequency; // Access frequency out of 100
 int size; // Size of item
 Shelf* assignedShelf;
 
 Item(string n, int freq, int s) 
 : name(n), frequency(freq), size(s), assignedShelf(nullptr) {}
};
// Structure to represent a location in the warehouse
struct Location {
 int x, y;
 string name;
 string coordinateName;
 
 Location(int x = 0, int y = 0, string name = "", string coordName = "") 
 : x(x), y(y), name(name), coordinateName(coordName) {}
 
 double distanceTo(const Location& other) const {
 return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
 }
};


// Structure for an item in the order
struct OrderItem {
 string itemName;
 int quantity;
 
 OrderItem(string name, int qty) : itemName(name), quantity(qty) {}
};
// Structure for an order
struct Order {
 int orderId;
 vector<OrderItem> items;
 bool isPrime;
 
 Order(int id, bool prime = false) : orderId(id), isPrime(prime) {}
 
 void addItem(string itemName, int quantity) {
 items.push_back(OrderItem(itemName, quantity));
 }
};
// Comparator for priority queue
struct OrderComparator {
 bool operator()(const Order& a, const Order& b) const {
 if (a.isPrime != b.isPrime)
 return !a.isPrime;
 return a.orderId > b.orderId;
 }
};
// Warehouse Management System Class
class WarehouseSystem {
private:
 Location entryPoint;
 vector<Shelf> shelves;
 unordered_map<string, Item*> items;
 unordered_map<string, Location> itemLocations;
 queue<Order> regularOrderQueue;
 priority_queue<Order, vector<Order>, OrderComparator> primeOrderQueue;
 
 const int HOT_ZONE_THRESHOLD = 60;
 const int FREQUENCY_INCREMENT = 2; // Increase by 2 when ordered
 const int FREQUENCY_DECREMENT = 1; // Decrease by 1 when not ordered
 
 string getCoordinateName(int x, int y) {
 char column = 'A' + x;
 int row = y + 1;
 return string(1, column) + to_string(row);
 }
 
 void initializeShelves() {
 // Hot Zone - 3x3 area near entry
 for (int y = 1; y <= 3; y++) {
 for (int x = 1; x <= 3; x++) {
 string coordName = getCoordinateName(x, y);
 shelves.push_back(Shelf(x, y, coordName, HOT_ZONE));
 }
 }
 
 // Cold Zone - rest of warehouse
 for (int y = 1; y <= 10; y++) {
 for (int x = 4; x <= 13; x++) {
 string coordName = getCoordinateName(x, y);
 shelves.push_back(Shelf(x, y, coordName, COLD_ZONE));
 }
 }
 
 for (int y = 4; y <= 10; y++) {
 for (int x = 1; x <= 3; x++) {
 string coordName = getCoordinateName(x, y);
 shelves.push_back(Shelf(x, y, coordName, COLD_ZONE));
 }
 }
 }
 
 Shelf* findBestShelf(int frequency, int itemSize) {
 ZoneType targetZone = (frequency >= HOT_ZONE_THRESHOLD) ? HOT_ZONE : 
COLD_ZONE;
 
 Shelf* bestShelf = nullptr;
 double minDistance = numeric_limits<double>::max();
 
 for (auto& shelf : shelves) {
 if (shelf.zone == targetZone && shelf.canStore(itemSize)) {
 double dist = shelf.distanceToEntry();
 if (dist < minDistance) {
 minDistance = dist;
 bestShelf = &shelf;
 }
 }
 }
 
 if (bestShelf == nullptr) {
 for (auto& shelf : shelves) {
 if (shelf.canStore(itemSize)) {
 double dist = shelf.distanceToEntry();
 if (dist < minDistance) {
 minDistance = dist;
 bestShelf = &shelf;
 }
 }
 }
 }
 
 return bestShelf;
 }
 
 vector<Location> generateWaypoints(Location from, Location to) {
 vector<Location> waypoints;
 int dx = to.x - from.x;
 int dy = to.y - from.y;
 int currentX = from.x;
 int currentY = from.y;
 
 int stepX = (dx > 0) ? 1 : -1;
 for (int i = 0; i < abs(dx); i++) {
 currentX += stepX;
 string coordName = getCoordinateName(currentX, currentY);
 waypoints.push_back(Location(currentX, currentY, "Waypoint", coordName));
 }
 
 int stepY = (dy > 0) ? 1 : -1;
 for (int i = 0; i < abs(dy); i++) {
 currentY += stepY;
 string coordName = getCoordinateName(currentX, currentY);
 waypoints.push_back(Location(currentX, currentY, "Waypoint", coordName));
 }
 
 return waypoints;
 }
 
 // Update frequencies based on order processing
 void updateFrequencies(const Order& order) {
 cout << "\n=== Updating Item Frequencies ===\n";
 
 // Collect ordered items
 unordered_map<string, bool> orderedItems;
 for (const auto& orderItem : order.items) {
 orderedItems[orderItem.itemName] = true;
 }
 
 // Update all items
 for (auto& pair : items) {
 Item* item = pair.second;
 int oldFreq = item->frequency;
 
 if (orderedItems.find(item->name) != orderedItems.end()) {
 // Item was ordered - increase frequency
 item->frequency = min(100, item->frequency + FREQUENCY_INCREMENT);
 if (item->frequency != oldFreq) {
 cout << " ↑ " << item->name << ": " << oldFreq << " -> " 
 << item->frequency << " (ordered)\n";
 }
 } else {
 // Item was not ordered - decrease frequency
 item->frequency = max(0, item->frequency - FREQUENCY_DECREMENT);
 if (item->frequency != oldFreq) {
 cout << " ↓ " << item->name << ": " << oldFreq << " -> " 
 << item->frequency << " (not ordered)\n";
 }
 }
 }
 }
 
 // Check and rebalance zones based on updated frequencies
 void rebalanceZones() {
 cout << "\n=== Checking Zone Assignments ===\n";
 
 vector<Item*> itemsToMove;
 
 // Check all items for zone mismatch
 for (auto& pair : items) {
 Item* item = pair.second;
 bool shouldBeHot = (item->frequency >= HOT_ZONE_THRESHOLD);
 bool isInHotZone = (item->assignedShelf->zone == HOT_ZONE);
 
 if (shouldBeHot && !isInHotZone) {
 cout << " ⚠ " << item->name << " (freq=" << item->frequency 
 << ") should move to HOT zone\n";
 itemsToMove.push_back(item);
 } else if (!shouldBeHot && isInHotZone) {
 cout << " ⚠ " << item->name << " (freq=" << item->frequency 
 << ") should move to COLD zone\n";
 itemsToMove.push_back(item);
 }
 }
 
 if (itemsToMove.empty()) {
 cout << " ✓ All items are in correct zones\n";
 return;
 }
 
 cout << "\n=== Rebalancing Zones ===\n";
 
 for (Item* item : itemsToMove) {
 // Remove from current shelf
 Shelf* oldShelf = item->assignedShelf;
 string oldLocation = oldShelf->coordinateName;
 string oldZone = (oldShelf->zone == HOT_ZONE) ? "HOT" : "COLD";
 oldShelf->removeItem();
 
 // Find new shelf
 Shelf* newShelf = findBestShelf(item->frequency, item->size);
 
 if (newShelf != nullptr) {
 newShelf->addItem(item->name, item->size);
 item->assignedShelf = newShelf;
 
 // Update location mapping
 Location loc(newShelf->x, newShelf->y, item->name, newShelf->coordinateName);
 itemLocations[item->name] = loc;
 
 string newZone = (newShelf->zone == HOT_ZONE) ? "HOT" : "COLD";
 cout << " ✓ Moved '" << item->name << "' from " << oldZone 
 << " (" << oldLocation << ") to " << newZone 
 << " (" << newShelf->coordinateName << ")\n";
 } else {
 // No space, put back
 oldShelf->addItem(item->name, item->size);
 cout << " ✗ No space to move '" << item->name << "'\n";
 }
 }
 }
 
public:
 WarehouseSystem(int entryX = 0, int entryY = 0) {
 string coordName = getCoordinateName(entryX, entryY);
 entryPoint = Location(entryX, entryY, "Entry Point", coordName);
 initializeShelves();
 cout << "Warehouse initialized with " << shelves.size() << " shelves\n";
 cout << "Hot Zone Threshold: Frequency >= " << HOT_ZONE_THRESHOLD << "\n";
 cout << "Each shelf: ONE item only, 1000 unit capacity\n";
 cout << "Frequency updates: +" << FREQUENCY_INCREMENT 
 << " when ordered, -" << FREQUENCY_DECREMENT << " when not\n";
 }
 
 void addItem(string itemName, int frequency, int size) {
 if (frequency < 0 || frequency > 100) {
 cout << "Error: Frequency must be between 0-100\n";
 return;
 }
 
 if (size <= 0 || size > 1000) {
 cout << "Error: Item size must be between 1-1000\n";
 return;
 }
 
 Shelf* bestShelf = findBestShelf(frequency, size);
 
 if (bestShelf == nullptr) {
 cout << "Error: No available shelf for " << itemName << "\n";
 return;
 }
 
 Item* newItem = new Item(itemName, frequency, size);
 newItem->assignedShelf = bestShelf;
 bestShelf->addItem(itemName, size);
 items[itemName] = newItem;
 
 Location loc(bestShelf->x, bestShelf->y, itemName, bestShelf->coordinateName);
 itemLocations[itemName] = loc;
 
 string zoneType = (bestShelf->zone == HOT_ZONE) ? "HOT ZONE" : "COLD ZONE";
 cout << "Added '" << itemName << "' (Freq: " << frequency << ", Size: " << size 
 << ") to " << zoneType << " at " << bestShelf->coordinateName << "\n";
 }
 
 void addOrder(const Order& order) {
 if (order.isPrime) {
 primeOrderQueue.push(order);
 cout << "Prime Order #" << order.orderId << " added to priority queue\n";
 } else {
 regularOrderQueue.push(order);
 cout << "Regular Order #" << order.orderId << " added to queue\n";
 }
 }
 
 Location* getItemLocation(string itemName) {
 if (itemLocations.find(itemName) != itemLocations.end()) {
 return &itemLocations[itemName];
 }
 return nullptr;
 }
 
 vector<Location> findOptimalPath(Location start, vector<Location> destinations) {
 vector<Location> optimalPath;
 optimalPath.push_back(start);
 
 Location currentPos = start;
 vector<bool> visited(destinations.size(), false);
 
 while (true) {
 double minDist = numeric_limits<double>::max();
 int nearestIdx = -1;
 
 for (int i = 0; i < destinations.size(); i++) {
 if (!visited[i]) {
 double dist = currentPos.distanceTo(destinations[i]);
 if (dist < minDist) {
 minDist = dist;
 nearestIdx = i;
 }
 }
 }
 
 if (nearestIdx == -1) break;
 
 visited[nearestIdx] = true;
 optimalPath.push_back(destinations[nearestIdx]);
 currentPos = destinations[nearestIdx];
 }
 
 optimalPath.push_back(start);
 return optimalPath;
 }
 
 double calculatePathDistance(const vector<Location>& path) {
 double totalDistance = 0.0;
 for (int i = 0; i < path.size() - 1; i++) {
 totalDistance += path[i].distanceTo(path[i + 1]);
 }
 return totalDistance;
 }
 
 void displayDetailedPath(const vector<Location>& path) {
 cout << "\n=== Detailed Picking Path ===\n";
 
 double totalTime = 0.0;
 double walkingSpeed = 2.0;
 
 for (int i = 0; i < path.size() - 1; i++) {
 Location from = path[i];
 Location to = path[i + 1];
 double segmentDist = from.distanceTo(to);
 double segmentTime = segmentDist / walkingSpeed;
 totalTime += segmentTime;
 
 cout << "\n" << (i + 1) << ". ";
 
 if (i == 0) {
 cout << "START: " << from.coordinateName << " (" << from.name << ")";
 } else if (i == path.size() - 2) {
 cout << "PICK ITEM: " << from.name << " at " << from.coordinateName;
 } else {
 cout << "PICK ITEM: " << from.name << " at " << from.coordinateName;
 }
 
 cout << "\n -> Going to " << to.coordinateName << " (" << to.name << ")";
 
 vector<Location> waypoints = generateWaypoints(from, to);
 
 if (!waypoints.empty()) {
 cout << "\n Path: " << from.coordinateName;
 for (const auto& wp : waypoints) {
 cout << " -> " << wp.coordinateName;
 }
 } else {
 cout << "\n Direct path: " << from.coordinateName << " -> " << to.coordinateName;
 }
 
 cout << "\n Distance: " << fixed << setprecision(2) << segmentDist << " units";
 cout << " | Time: " << fixed << setprecision(2) << segmentTime << " mins\n";
 }
 
 cout << "\n" << string(50, '=') << "\n";
 cout << "TOTAL DISTANCE: " << fixed << setprecision(2) 
 << calculatePathDistance(path) << " units\n";
 cout << "ESTIMATED TOTAL TIME: " << fixed << setprecision(2) 
 << totalTime << " minutes\n";
 cout << string(50, '=') << "\n";
 }
 
 void displayPathOnMap(const vector<Location>& path) {
 cout << "\n=== Path Visualization on Map ===\n";
 
 int maxX = 13, maxY = 10;
 vector<vector<string>> grid(maxY + 2, vector<string>(maxX + 2, "."));
 
 // Mark path
 for (int i = 0; i < path.size(); i++) {
 if (i == 0) {
 grid[path[i].y][path[i].x] = "S";
 } else if (i == path.size() - 1) {
 grid[path[i].y][path[i].x] = "E";
 } else {
 grid[path[i].y][path[i].x] = to_string(i);
 }
 }
 
 cout << " ";
 for (int x = 0; x <= maxX; x++) {
 char col = 'A' + x;
 cout << " " << col;
 }
 cout << "\n";
 
 for (int y = maxY; y >= 0; y--) {
 cout << setw(3) << (y + 1) << " ";
 for (int x = 0; x <= maxX; x++) {
 cout << setw(3) << grid[y][x];
 }
 cout << "\n";
 }
 
 cout << "\nLegend:\n";
 cout << " S = Start (Entry)\n";
 cout << " 1-9 = Pick sequence\n";
 cout << " E = End/Return\n";
 cout << " . = Empty\n";
 }
 
 void processNextOrder() {
 Order currentOrder(0);
 bool hasOrder = false;
 
 if (!primeOrderQueue.empty()) {
 currentOrder = primeOrderQueue.top();
 primeOrderQueue.pop();
 hasOrder = true;
 cout << "\n" << string(60, '=') << "\n";
 cout << " Processing PRIME Order #" << currentOrder.orderId << "\n";
 cout << string(60, '=') << "\n";
 } else if (!regularOrderQueue.empty()) {
 currentOrder = regularOrderQueue.front();
 regularOrderQueue.pop();
 hasOrder = true;
 cout << "\n" << string(60, '=') << "\n";
 cout << " Processing Regular Order #" << currentOrder.orderId << "\n";
 cout << string(60, '=') << "\n";
 }
 
 if (!hasOrder) {
 cout << "\nNo orders to process!\n";
 return;
 }
 
 vector<Location> destinations;
 cout << "\nItems in this order:\n";
 
 for (const auto& item : currentOrder.items) {
 Location* loc = getItemLocation(item.itemName);
 if (loc != nullptr) {
 destinations.push_back(*loc);
 auto it = items.find(item.itemName);
 string zone = (it != items.end() && it->second->assignedShelf->zone == HOT_ZONE) 
 ? "[HOT]" : "[COLD]";
 cout << " + " << item.itemName << " (Qty: " << item.quantity 
 << ") at " << loc->coordinateName << " " << zone << "\n";
 } else {
 cout << " - " << item.itemName << " - NOT FOUND!\n";
 }
 }
 
 if (destinations.empty()) {
 cout << "\nNo valid items. Order cannot be fulfilled.\n";
 return;
 }
 
 cout << "\nCalculating optimal route...\n";
 vector<Location> optimalPath = findOptimalPath(entryPoint, destinations);
 
 displayDetailedPath(optimalPath);
 displayPathOnMap(optimalPath);
 
 cout << "\n[OK] Order #" << currentOrder.orderId << " completed!\n";
 
 // Update frequencies and rebalance zones
 updateFrequencies(currentOrder);
 rebalanceZones();
 }
 
 void displayWarehouseLayout() {
 cout << "\n=== Warehouse Layout Matrix ===\n";
 
 int maxX = 13, maxY = 10;
 vector<vector<string>> grid(maxY + 2, vector<string>(maxX + 2, "."));
 
 // Mark entry point
 grid[entryPoint.y][entryPoint.x] = "E";
 
 // Map item names to codes
 unordered_map<string, int> itemCodes;
 int codeNum = 1;
 
 for (const auto& shelf : shelves) {
 if (!shelf.isEmpty()) {
 if (itemCodes.find(shelf.itemStored) == itemCodes.end()) {
 itemCodes[shelf.itemStored] = codeNum++;
 }
 grid[shelf.y][shelf.x] = to_string(itemCodes[shelf.itemStored]);
 }
 }
 
 // Print column headers
 cout << " ";
 for (int x = 0; x <= maxX; x++) {
 char col = 'A' + x;
 cout << setw(3) << col;
 }
 cout << "\n";
 
 // Print grid
 for (int y = maxY; y >= 0; y--) {
 cout << setw(4) << (y + 1) << " ";
 for (int x = 0; x <= maxX; x++) {
 cout << setw(3) << grid[y][x];
 }
 cout << "\n";
 }
 
 // Print zone information
 cout << "\n=== Zone Configuration ===\n";
 
 vector<string> hotZoneShelves;
 vector<string> coldZoneShelves;
 
 for (const auto& shelf : shelves) {
 if (shelf.zone == HOT_ZONE) {
 hotZoneShelves.push_back(shelf.coordinateName);
 } else {
 coldZoneShelves.push_back(shelf.coordinateName);
 }
 }
 
 cout << "\nHot Zone Shelves (Frequency >= " << HOT_ZONE_THRESHOLD << "):\n ";
 for (size_t i = 0; i < hotZoneShelves.size(); i++) {
 cout << hotZoneShelves[i];
 if (i < hotZoneShelves.size() - 1) cout << ", ";
 if ((i + 1) % 10 == 0) cout << "\n ";
 }
 
 cout << "\n\nAll other shelves are Cold Zone (Frequency < " << HOT_ZONE_THRESHOLD 
<< ")\n";
 
 // Print legend
 cout << "\n=== Legend ===\n";
 cout << " E = Entry Point (" << entryPoint.coordinateName << ")\n";
 cout << " 1-9 = Item code (see table below)\n";
 cout << " . = Empty shelf or space\n";
 
 if (!itemCodes.empty()) {
 cout << "\n=== Items on Shelves ===\n";
 cout << left << setw(5) << "Code" << setw(20) << "Item" 
 << setw(12) << "Location" << setw(10) << "Zone" 
 << setw(10) << "Freq" << "Size\n";
 cout << string(70, '-') << "\n";
 
 vector<pair<string, int>> sortedItems;
 for (const auto& pair : itemCodes) {
 sortedItems.push_back(pair);
 }
 sort(sortedItems.begin(), sortedItems.end(), 
 [](const auto& a, const auto& b) { return a.second < b.second; });
 
 for (const auto& pair : sortedItems) {
 string itemName = pair.first;
 int code = pair.second;
 
 auto it = items.find(itemName);
 if (it != items.end()) {
 Item* item = it->second;
 string zone = (item->assignedShelf->zone == HOT_ZONE) ? "HOT" : "COLD";
 cout << left << setw(5) << code 
 << setw(20) << itemName
 << setw(12) << item->assignedShelf->coordinateName
 << setw(10) << zone
 << setw(10) << item->frequency
 << item->size << "\n";
 }
 }
 }
 }
 
 void displayZones() {
 cout << "\n=== Warehouse Zones Summary ===\n";
 
 int hotCount = 0, coldCount = 0;
 int hotItemCount = 0, coldItemCount = 0;
 
 for (const auto& shelf : shelves) {
 if (shelf.zone == HOT_ZONE) {
 hotCount++;
 if (!shelf.isEmpty()) hotItemCount++;
 } else {
 coldCount++;
 if (!shelf.isEmpty()) coldItemCount++;
 }
 }
 
 cout << "\nHot Zone (Frequency >= " << HOT_ZONE_THRESHOLD << "):\n";
 cout << " Total Shelves: " << hotCount << "\n";
 cout << " Occupied: " << hotItemCount << "\n";
 cout << " Available: " << (hotCount - hotItemCount) << "\n";
 
 cout << "\nCold Zone (Frequency < " << HOT_ZONE_THRESHOLD << "):\n";
 cout << " Total Shelves: " << coldCount << "\n";
 cout << " Occupied: " << coldItemCount << "\n";
 cout << " Available: " << (coldCount - coldItemCount) << "\n";
 }
 
 void displayInventory() {
 cout << "\n=== Warehouse Inventory ===\n";
 cout << "Entry Point: " << entryPoint.coordinateName << "\n\n";
 
 cout << left << setw(20) << "Item" << setw(10) << "Frequency" 
 << setw(8) << "Size" << setw(12) << "Location" 
 << setw(10) << "Zone" << "\n";
 cout << string(60, '-') << "\n";
 
 for (const auto& pair : items) {
 Item* item = pair.second;
 string zone = (item->assignedShelf->zone == HOT_ZONE) ? "HOT" : "COLD";
 cout << left << setw(20) << item->name 
 << setw(10) << item->frequency 
 << setw(8) << item->size
 << setw(12) << item->assignedShelf->coordinateName
 << setw(10) << zone << "\n";
 }
 }
 
 void displayQueues() {
 cout << "\n=== Order Status ===\n";
 cout << "Prime Orders: " << primeOrderQueue.size() << "\n";
 cout << "Regular Orders: " << regularOrderQueue.size() << "\n";
 }
 
 bool hasPendingOrders() {
 return !primeOrderQueue.empty() || !regularOrderQueue.empty();
 }
};
void displayMenu() {
 cout << "\n========================================\n";
 cout << " WAREHOUSE MANAGEMENT SYSTEM - MENU\n";
 cout << "========================================\n";
 cout << "1. Add Item (with frequency)\n";
 cout << "2. Create New Order\n";
 cout << "3. Process Next Order\n";
 cout << "4. Display Warehouse Layout (Matrix)\n";
 cout << "5. Display Warehouse Zones Summary\n";
 cout << "6. Display Inventory Details\n";
 cout << "7. Display Order Queues\n";
 cout << "8. Search Item Location\n";
 cout << "9. Process All Orders\n";
 cout << "10. Load Sample Data\n";
 cout << "0. Exit\n";
 cout << "========================================\n";
 cout << "Enter your choice: ";
}
void loadSampleData(WarehouseSystem& warehouse) {
 cout << "\nLoading sample data...\n\n";
 
 warehouse.addItem("Laptop", 85, 150);
 warehouse.addItem("Mouse", 90, 50);
 warehouse.addItem("Keyboard", 75, 100);
 warehouse.addItem("USB Cable", 80, 30);
 warehouse.addItem("Monitor", 55, 200);
 warehouse.addItem("Headphones", 50, 80);
 warehouse.addItem("Webcam", 40, 100);
 warehouse.addItem("HDMI Cable", 35, 40);
 warehouse.addItem("Printer", 25, 300);
 warehouse.addItem("Scanner", 20, 250);
 
 Order order1(1, false);
 order1.addItem("Laptop", 1);
 order1.addItem("Mouse", 2);
 warehouse.addOrder(order1);
 
 Order order2(2, true);
 order2.addItem("Keyboard", 1);
 order2.addItem("Monitor", 1);
 warehouse.addOrder(order2);
 
 Order order3(3, false);
 order3.addItem("Printer", 1);
 order3.addItem("Scanner", 1);
 warehouse.addOrder(order3);
 
 cout << "\nSample data loaded!\n";
}
int main() {
 cout << "========================================\n";
 cout << " WAREHOUSE MANAGEMENT SYSTEM\n";
 cout << " with Dynamic Zone Rebalancing\n";
 cout << "========================================\n\n";
 
 WarehouseSystem warehouse(0, 0);
 int choice, orderCounter = 1;
 bool running = true;
 
 while (running) {
 displayMenu();
 cin >> choice;
 cin.ignore(numeric_limits<streamsize>::max(), '\n');
 
 switch (choice) {
 case 1: {
 string itemName;
 int frequency, size;
 cout << "\nEnter item name: ";
 getline(cin, itemName);
 cout << "Enter access frequency (0-100): ";
 cin >> frequency;
 cout << "Enter item size (1-1000): ";
 cin >> size;
 cin.ignore();
 warehouse.addItem(itemName, frequency, size);
 break;
 }
 
 case 2: {
 char isPrime;
 int numItems;
 cout << "\nIs this a Prime order? (y/n): ";
 cin >> isPrime;
 bool prime = (isPrime == 'y' || isPrime == 'Y');
 Order newOrder(orderCounter++, prime);
 cout << "How many items? ";
 cin >> numItems;
 cin.ignore();
 
 for (int i = 0; i < numItems; i++) {
 string itemName;
 int quantity;
 cout << " Item " << (i + 1) << " name: ";
 getline(cin, itemName);
 cout << " Quantity: ";
 cin >> quantity;
 cin.ignore();
 newOrder.addItem(itemName, quantity);
 }
 warehouse.addOrder(newOrder);
 break;
 }
 
 case 3: {
 warehouse.processNextOrder();
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 4: {
 warehouse.displayWarehouseLayout();
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 5: {
 warehouse.displayZones();
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 6: {
 warehouse.displayInventory();
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 7: {
 warehouse.displayQueues();
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 8: {
 string itemName;
 cout << "\nEnter item name: ";
 getline(cin, itemName);
 Location* loc = warehouse.getItemLocation(itemName);
 if (loc != nullptr) {
 cout << "\nFound at " << loc->coordinateName << "\n";
 } else {
 cout << "\nNot found!\n";
 }
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 9: {
 cout << "\nProcessing all orders...\n";
 while (warehouse.hasPendingOrders()) {
 warehouse.processNextOrder();
 if (warehouse.hasPendingOrders()) {
 cout << "\nPress Enter for next...";
 cin.get();
 }
 }
 cout << "\nAll orders done!\n";
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 10: {
 loadSampleData(warehouse);
 orderCounter = 4;
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 
 case 0: {
 cout << "\nThank you! Goodbye!\n";
 running = false;
 break;
 }
 
 default: {
 cout << "\nInvalid choice!\n";
 cout << "\nPress Enter...";
 cin.get();
 break;
 }
 }
 }
 return 0;
}

