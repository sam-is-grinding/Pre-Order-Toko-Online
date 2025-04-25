#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h" 
#include "rapidjson/prettywriter.h"
#include <deque>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <stack>

#define CLEAR_SCREEN "\033[2J\033[1;1H"

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
unordered_map<string, string> produkListById;
unordered_map<string, string> idListByProduk;
unordered_map<string, shared_ptr<Order>> orderListById;
unordered_map<string, vector<shared_ptr<Order>>> orderListByProduk;
stack<pair<shared_ptr<Order>, int>> undoStack;


// declarations
rapidjson::Document loadJsonFile(const char* path);
void loadProdukFromJson(const char *path);
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
			 << ", Barang: " << produkListById.find(orderPtr->getIdProduk())->second
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
		cout << "Order dengan ID [" << idOrder << "] ditemukan!\n";
        cout << "Barang: " << produkListById.find(orderPtr->getIdProduk())->second
             << ", Tanggal Order: " << orderPtr->getTanggal()
			 << ", Kuantitas: " << orderPtr->getKuantitas()
			 << "\n";
        cout << endl;
    } else {
        cout << "Order dengan ID [" << idOrder << "] tidak ditemukan.\n";
        cout << endl;
    }
}

void printOrderByProduk(string namaProduk) {
	auto it = orderListByProduk.find(idListByProduk.find(namaProduk)->second);
    if (it != orderListByProduk.end()) {
		// sort bedasarkan tanggal
		sort(it->second.begin(), it->second.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
			return a->getTanggal() < b->getTanggal();
		});
		cout << "Order dengan produk [" << namaProduk << "] ditemukan!\n";
		for (auto& orderPtr : it->second) {
			cout << "Order: " << orderPtr->getIdOrder()
				 << ", Tanggal Order: " << orderPtr->getTanggal()
				 << ", Kuantitas: " << orderPtr->getKuantitas()
				 << "\n";
        }
	   cout << endl;
    } else {
        cout << "Order dengan produk [" << namaProduk << "] tidak ditemukan.\n";
		cout << endl;
    }
}


void batalkanPesanan(string idOrder) {
    for (int i = 0; i < orderQueue.size(); i++) {
        if (orderQueue[i]->getIdOrder() == idOrder) {
            undoStack.push({orderQueue[i], i});
            orderQueue.erase(orderQueue.begin() + i);
            cout << "Order dengan ID [" << idOrder << "] berhasil dihapus.\n";
            cout << endl;
            return;
        }
    }
    cout << "PEMBATALAN GAGAL. Order dengan ID [" << idOrder << "] tidak ditemukan.\n";
    cout << endl;
}

void undoPembatalan() {
    if (!undoStack.empty()) {
        auto lastDeleted = undoStack.top();
        orderQueue.insert(orderQueue.begin() + lastDeleted.second, lastDeleted.first);
        cout << "Undo-pembatalan order dengan ID [" << lastDeleted.first->getIdOrder() << "] berhasil.\n" << endl;
        undoStack.pop();
    } else {
        cout << "UNDO-PEMBATALAN GAGAL. Tidak ada yang bisa diundo." << endl;
    }
}



void printGreetings() {
    cout << R"(
 ___       ___   ___   ___   ____  ___      
| |_)     / / \ | |_) | | \ | |_  | |_) 
|_|       \_\_/ |_| \ |_|_/ |_|__ |_| \.)" << endl;

         std::cout << R"(
         +------------------------------------------------------------+
         |                    ORDER COMMANDS                          |
         +------------------------------------------------------------+
         | [list (n)]           : list semua order                    |
         | [produk (nama) (n)]  : list order berdasarkan NAMA produk  |
         | [order (id) (n)]     : list order berdasarkan ID order     |
         | [batal (id)]         : batalkan order                      |
         | [undo]               : undo pembatalkan order terakhir     |
         | [exit]               : keluar dari program                 |
         +------------------------------------------------------------+
         | Sorting options (n):                                       |
         |   - at : ascending tanggal                                 |
         |   - dt : descending tanggal                                |
         |   - ak : ascending kuantitas                               |
         |   - dk : descending kuantitas                              |
         +------------------------------------------------------------+
         
         )" << std::endl;
}

void toUpper(string& s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return toupper(c); });
}


int main() {
	// load produk & order dari .json
    loadProdukFromJson("list-produk.json");
	loadOrderFromJson("list-order.json");
	

	// sort orderQueue berdasarkan tanggal
	sort(orderQueue.begin(), orderQueue.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
		return a->getTanggal() < b->getTanggal();
	});

    cout << CLEAR_SCREEN;
    printGreetings();

    string input;
    string command;
    string args;
    string option;
    // baca input
    cout << "Masukkan perintah: ";
    getline(cin, input);
    toUpper(input);
    stringstream iss(input);
    iss >> command;
    while (command != "EXIT") {
        cout << CLEAR_SCREEN;
        printGreetings();

        if (command == "LIST") {
            iss >> option;  // kosong kalo gaada -n
            printOrder();
        } else if (command == "PRODUK") {
            iss >> args;
            iss >> option;
            printOrderByProduk(args);
        } else if (command == "ORDER") {
            iss >> args;
            iss >> option;
            printOrderById(args);
        } else if (command == "BATAL") {
            iss >> args;
            batalkanPesanan(args);
        } else if (command == "UNDO") {
            undoPembatalan();
        } else {
            cout << "gaada beruh" << endl;
            cout << command << endl;
        }
        
        // baca input lagi
        cout << "Masukkan perintah: ";
        getline(cin, input);
        toUpper(input);
        iss.clear();
        iss.str(input);
        iss >> command;
        }
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

void loadProdukFromJson(const char *path) {
	rapidjson::Document doc = loadJsonFile(path);
    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        const std::string key = it->name.GetString();
        const std::string value = it->value.GetString();
        produkListById[key] = value;
        idListByProduk[value] = key;
    }
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