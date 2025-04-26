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


enum class StatusOrder {
    Aktif,
    Dibatalkan,
    Selesai,
    Invalid
};

string statusToSting(StatusOrder status) {
    switch (status) {
        case StatusOrder::Aktif: return "Aktif";
        case StatusOrder::Dibatalkan: return "Dibatalkan";
        case StatusOrder::Selesai: return "Selesai";
        default: return "lol";
    }
}

StatusOrder stringToStatus(const string& str) {
    if (str == "Aktif") return StatusOrder::Aktif;
    if (str == "Dibatalkan") return StatusOrder::Dibatalkan;
    if (str == "Selesai") return StatusOrder::Selesai;
    return StatusOrder::Invalid;
}


class Order {
	string idOrder;
    string idProduk;
    string tanggal;
    StatusOrder status;
	int kuantitas;
public:
    Order (string idOrder, string idProduk, string tanggal, int kuantitas, StatusOrder status) : idOrder(idOrder), idProduk(idProduk), tanggal(tanggal), kuantitas(kuantitas), status(status) {}
    string getIdOrder() { return idOrder; }
	string getIdProduk() { return idProduk; }
	string getTanggal() { return tanggal; }
	int getKuantitas() { return kuantitas; }
    StatusOrder getStatus() {return status; }
    void setStatus(StatusOrder s) {status = s; }
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

void sortOrders(vector<std::shared_ptr<Order>>& orders, string option) {
    if (option == "") { // kosong, default ke sort ascending tanggal
        sort(orders.begin(), orders.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
            return a->getTanggal() < b->getTanggal();
        });
	    cout << "Opsi sorting kosong. Order disort secara ascending berdasarkan tanggal.\n";
    } else if (option == "AT") { // sort ascending tanggal
        sort(orders.begin(), orders.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
            return a->getTanggal() < b->getTanggal();
        });
	    cout << "Sorting ascending berdasarkan tanggal berhasil!\n";
    } else if (option == "DT") { // sort descending tanggal
        sort(orders.begin(), orders.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
            return a->getTanggal() > b->getTanggal();
        });
	    cout << "Sorting descending berdasarkan tanggal berhasil!\n";
    } else if (option == "AK") { // sort ascending kuantitas
        sort(orders.begin(), orders.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
            return a->getKuantitas() < b->getKuantitas();
        });
	    cout << "Sorting ascending berdasarkan kuantitas berhasil!\n";
    } else if (option == "DK") { // sort descending kuantitas
        sort(orders.begin(), orders.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
            return a->getKuantitas() > b->getKuantitas();
        });
	    cout << "Sorting descending berdasarkan kuantitas berhasil!\n";
    } else {
	    cout << "SORTING GAGAL. Opsi sort berdasarkan [" << option << "] tidak tersedia.\n";
        cout << option << endl;
    }
}

void printDaftarTunggu(string option) {
    vector<shared_ptr<Order>> orders(orderQueue.begin(), orderQueue.end());
    sortOrders(orders, option);

	cout << "List order:\n";
	for (auto& orderPtr : orders) {
        cout << "Order: " << orderPtr->getIdOrder()
			 << ", Barang: " << produkListById.find(orderPtr->getIdProduk())->second
			 << ", Tanggal Order: " << orderPtr->getTanggal()
			 << ", Kuantitas: " << orderPtr->getKuantitas()
             << ", Status: " << statusToSting(orderPtr->getStatus())
			 << "\n";
	}
	cout << endl;
}

void printSemuaOrder(string option) {
    vector<shared_ptr<Order>> orders;

    for (const auto& [id, orderPtr] : orderListById) {
        orders.push_back(orderPtr);
    }
    sortOrders(orders, option);

	cout << "List order:\n";
    for (auto& orderPtr : orders) {
        cout << "Order: " << orderPtr->getIdOrder()
			 << ", Barang: " << produkListById.find(orderPtr->getIdProduk())->second
			 << ", Tanggal Order: " << orderPtr->getTanggal()
			 << ", Kuantitas: " << orderPtr->getKuantitas()
             << ", Status: " << statusToSting(orderPtr->getStatus())
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
			 << ", Status: " << statusToSting(orderPtr->getStatus())
			 << "\n";
        cout << endl;
    } else {
        cout << "Order dengan ID [" << idOrder << "] tidak ditemukan.\n";
        cout << endl;
    }
}


void printOrderByProduk(string namaProduk) {
    auto _it = idListByProduk.find(namaProduk);
    if (_it != idListByProduk.end()) {
        auto it = orderListByProduk.find(_it->second);
        //sort bedasarkan tanggal
		sort(it->second.begin(), it->second.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
			return a->getTanggal() < b->getTanggal();
        });
        cout << "Order dengan produk [" << namaProduk << "] ditemukan!\n";
        for (auto& orderPtr : it->second) {
            cout << "Order: " << orderPtr->getIdOrder()
                << ", Tanggal Order: " << orderPtr->getTanggal()
                << ", Kuantitas: " << orderPtr->getKuantitas()
                << ", Status: " << statusToSting(orderPtr->getStatus())
                << "\n";
        }
    } else {
        cout << "Order dengan produk [" << namaProduk << "] tidak ditemukan.\n";
		cout << endl;
    }
}


void batalkanPesanan(string idOrder) {
    for (int i = 0; i < orderQueue.size(); i++) {
        shared_ptr<Order> order = orderQueue[i];
        if (order->getIdOrder() == idOrder) {
            order->setStatus(StatusOrder::Dibatalkan);
            undoStack.push({order, i});
            orderQueue.erase(orderQueue.begin() + i);
            cout << "Order dengan ID [" << idOrder << "] berhasil dibatalkan.\n";
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
        shared_ptr<Order> order = lastDeleted.first;
        order->setStatus(StatusOrder::Aktif);
        orderQueue.insert(orderQueue.begin() + lastDeleted.second, order);
        cout << "Undo-pembatalan order dengan ID [" << order->getIdOrder() << "] berhasil.\n" << endl;
        undoStack.pop();
    } else {
        cout << "UNDO-PEMBATALAN GAGAL. Tidak ada yang bisa diundo.\n" << endl;
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
         | [daftar (n)]         : lihat daftar tunggu                 |
         | [semua (n)]          : lihat semua order                   |
         | [produk (nama) (n)]  : lihat order berdasarkan NAMA produk |
         | [order (id) (n)]     : lihat order berdasarkan ID order    |
         | [batal (id)]         : batalkan order                      |
         | [undo]               : undo pembatalkan order terakhir     |
         | [exit]               : keluar dari program                 |
         +------------------------------------------------------------+
         | Sorting options (n):                                       |
         |      at : ascending tanggal                                |
         |      dt : descending tanggal                               |
         |      ak : ascending kuantitas                              |
         |      dk : descending kuantitas                             |
         +------------------------------------------------------------+
         
         )" << std::endl;
}

void toUpper(string& s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return toupper(c); });
}


int main() {
	// load produk & order dari .json
    loadProdukFromJson("list-produk.json");
	loadOrderFromJson("list-order-awal.json");
    
	// // sort orderQueue berdasarkan tanggal
	// sort(orderQueue.begin(), orderQueue.end(), [](const shared_ptr<Order>& a, const shared_ptr<Order>& b) {
    //     return a->getTanggal() < b->getTanggal();
	// });


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

        if (command == "DAFTAR") {
            iss >> option;  // kosong kalo gaada -n
            printDaftarTunggu(option);
        } else if (command == "SEMUA") {
            iss >> option;  // kosong kalo gaada -n
            printSemuaOrder(option);
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
    
    saveOrderToJson("list-order.json");
}



rapidjson::Document loadJsonFile(const char *path) {
	FILE *fp = fopen(path, "rb");
	// if (!fp) {
	// 	throw runtime_error("Failed to open file");
	// }

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document doc;
	doc.ParseStream(is);

	fclose(fp);

	// if (doc.HasParseError()) {
	// 	throw runtime_error("Failed to parse JSON");
	// }

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
		string idOrder      = order["idOrder"].GetString();
		string idProduk     = order["idProduk"].GetString();
		string tanggal      = order["tanggal"].GetString();
		int kuantitas       = order["kuantitas"].GetInt();
        StatusOrder status  = stringToStatus(order["status"].GetString()); 
		registerOrder(make_shared<Order>(idOrder, idProduk, tanggal, kuantitas, status));
	}
}


void saveOrderToJson(const char* path) {
    rapidjson::Document doc;
    doc.SetArray();
    auto& allocator = doc.GetAllocator();

    for (auto& [id, orderPtr] : orderListById) {
        Order& order = *orderPtr;

        rapidjson::Value obj(rapidjson::kObjectType);
		obj.AddMember("idOrder", rapidjson::Value().SetString(order.getIdOrder().c_str(), allocator), allocator);
		obj.AddMember("idProduk", rapidjson::Value().SetString(order.getIdProduk().c_str(), allocator), allocator);
		obj.AddMember("tanggal", rapidjson::Value().SetString(order.getTanggal().c_str(), allocator), allocator);
		obj.AddMember("kuantitas", order.getKuantitas(), allocator);
        obj.AddMember("status", rapidjson::Value().SetString(statusToSting(order.getStatus()).c_str(), allocator), allocator);

        doc.PushBack(obj, allocator);
    }

    FILE* fp = fopen(path, "wb");
    // if (!fp) throw runtime_error("Failed to open file");

    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(fp);
}