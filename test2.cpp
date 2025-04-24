#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h" 
#include "rapidjson/prettywriter.h"
#include <cstdio>	 // for FILE*, fopen, fclose
#include <stdexcept> // for runtime_error
#include <deque>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>


using namespace std;


class Order {
	string idOrder;
    string idProduk;
    string tanggal;
	int kuantitas;
public:
    Order (string idOrder, string idProduk, string tanggal, int kuantitas) : idOrder(idOrder), idProduk(idProduk), tanggal(tanggal), kuantitas(kuantitas) {}
    string getIdOrder() { return idOrder; }
	string getIdProduk() { return idProduk; }
	string getTanggal() { return tanggal; }
	int getKuantitas() { return kuantitas; }
};

// globals
deque<shared_ptr<Order>> orderQueue;
unordered_map<string, shared_ptr<Order>> orderListById;
unordered_map<string, vector<shared_ptr<Order>>> orderListByProduk;


// declarations
rapidjson::Document loadJsonFile(const char* path);
void loadOrderFromJson(const char *path);
void saveOrderToJson(const char* path);

void registerOrder(shared_ptr<Order> order) {
	orderQueue.push_back(order);		
	orderListById[order->getIdOrder()] = order;
	orderListByProduk[order->getIdProduk()].push_back(order);	
}

void printOrder() {
	cout << "List order:\n";
	for (auto& orderPtr : orderQueue) {
        cout << "Order: " << orderPtr->getIdOrder()
			 << ", Barang: " << orderPtr->getIdProduk()
			 << ", Tanggal Order: " << orderPtr->getTanggal()
			 << ", Kuantitas: " << orderPtr->getKuantitas()
			 << "\n";
	}
	cout << endl;
}

void printOrderById(string idOrder) {
	auto it = orderListById.find(idOrder);
    if (it != orderListById.end()) {
        auto& orderPtr = it->second;
		cout << "Order dengan ID " << idOrder << " ditemukan!\n";
        cout << "Barang: " << orderPtr->getIdProduk()
             << ", Tanggal Order: " << orderPtr->getTanggal()
			 << ", Kuantitas: " << orderPtr->getKuantitas()
			 << "\n";
        cout << endl;
    } else {
        cout << "Order dengan ID " << idOrder << " tidak ditemukan.\n";
        cout << endl;
    }
}

void printOrderByProduk(string idProduk) {
	auto it = orderListByProduk.find(idProduk);
    if (it != orderListByProduk.end()) {
		// sort bedasarkan tanggal
		sort(it->second.begin(), it->second.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
			return a->getTanggal() < b->getTanggal();
		});
		cout << "Produk dengan ID " << idProduk << " ditemukan!\n";
		for (auto& orderPtr : it->second) {
			cout << "Order: " << orderPtr->getIdOrder()
				 << ", Tanggal Order: " << orderPtr->getTanggal()
				 << ", Kuantitas: " << orderPtr->getKuantitas()
				 << "\n";
        }
	   cout << endl;
    } else {
        cout << "Produk dengan ID " << idProduk << " tidak ditemukan.\n";
		cout << endl;
    }
}



int main() {
	// registerOrder(make_shared<Order>("ORD005", "2025-04-02", "P001", 1));
	// registerOrder(make_shared<Order>("ORD007", "2025-05-28", "P002", 2));
	// registerOrder(make_shared<Order>("ORD001", "2025-03-07", "P001", 1));
	// registerOrder(make_shared<Order>("ORD004", "2025-04-01", "P099", 4));
	// registerOrder(make_shared<Order>("ORD006", "2025-05-27", "P001", 3));
	// registerOrder(make_shared<Order>("ORD002", "2025-03-08", "P099", 1));

	// load order dari list-order.json
	loadOrderFromJson("list-order.json");
	

	// sort orderQueue berdasarkan tanggal
	sort(orderQueue.begin(), orderQueue.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
		return a->getTanggal() < b->getTanggal();
	});

	// saveOrderToJson("list-order.json");


	printOrder();
	printOrderById("ORD002");

	printOrderByProduk("P001");
	printOrderByProduk("P004");
	printOrderByProduk("P099");
}



rapidjson::Document loadJsonFile(const char *path) {
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		throw runtime_error("Failed to open file");
	}

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document doc;
	doc.ParseStream(is);

	fclose(fp);

	if (doc.HasParseError()) {
		throw runtime_error("Failed to parse JSON");
	}

	return doc;
}

void loadOrderFromJson(const char *path) {
	rapidjson::Document doc = loadJsonFile(path);
	for (const auto& order : doc.GetArray()) {
		string idOrder = order["idOrder"].GetString();
		string idProduk = order["idProduk"].GetString();
		string tanggal  = order["tanggal"].GetString();
		int kuantitas   = order["kuantitas"].GetInt();
		registerOrder(make_shared<Order>(idOrder, idProduk, tanggal, kuantitas));
	}
}


void saveOrderToJson(const char* path) {
    rapidjson::Document doc;
    doc.SetArray();
    auto& allocator = doc.GetAllocator();

    for (auto& orderPtr : orderQueue) {
        Order& order = *orderPtr;

        rapidjson::Value obj(rapidjson::kObjectType);
		obj.AddMember("idOrder", rapidjson::Value().SetString(order.getIdOrder().c_str(), allocator), allocator);
		obj.AddMember("idProduk", rapidjson::Value().SetString(order.getIdProduk().c_str(), allocator), allocator);
		obj.AddMember("tanggal", rapidjson::Value().SetString(order.getTanggal().c_str(), allocator), allocator);
		obj.AddMember("kuantitas", order.getKuantitas(), allocator);

        doc.PushBack(obj, allocator);
    }

    FILE* fp = fopen(path, "wb");
    if (!fp) throw runtime_error("Failed to open file");

    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(fp);
}
