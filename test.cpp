#include <iostream>
#include <deque>
#include <stack>
#include <string>

using namespace std;


class Pesanan {
    string idPesanan;
    string tanggal;
    string idProduk;
public:
    Pesanan (string idpesanan, string tanggal, string idproduk) : idPesanan(idpesanan), tanggal(tanggal), idProduk(idproduk) {}
    string getIdPesanan() {
        return idPesanan;
    }
};

stack<pair<Pesanan, int>> undoStack;

void batalkanPesanan(deque<Pesanan>& pesananQueue, const string& idPesanan) {
    for (int i = 0; i < pesananQueue.size(); i++) {
        if (pesananQueue[i].getIdPesanan() == idPesanan) {
            undoStack.push({pesananQueue[i], i});
            pesananQueue.erase(pesananQueue.begin() + i);
            cout << "Pesanan " << idPesanan << " dihapus." << endl;
            return;
        }
    }
    cout << "Pesanan " << idPesanan << " tidak ditemukan" << endl;
}

void undoPembatalan(deque<Pesanan>& pesananQueue) {
    if (!undoStack.empty()) {
        auto lastDeleted = undoStack.top();
        pesananQueue.insert(pesananQueue.begin() + lastDeleted.second, lastDeleted.first);
        cout << "Undo pesanan " << lastDeleted.first.getIdPesanan() << " berhasil" << endl;
        undoStack.pop();
    } else {
        cout << "Tidak ada yang bisa diundo." << endl;
    }
}


void printPesanan(deque<Pesanan> &pesanan) {
    for (Pesanan o : pesanan) {
        cout << o.getIdPesanan() << "\n";
      }

      cout << endl;
}



int main() {
    deque<Pesanan> pesananQueue;
    pesananQueue.push_back(Pesanan("ORD001", "2025-03-07", "P001"));
    pesananQueue.push_back(Pesanan("ORD002", "2025-03-05", "P002"));
    pesananQueue.push_back(Pesanan("ORD003", "2025-03-07", "P003"));
    pesananQueue.push_back(Pesanan("ORD004", "2025-03-06", "P001"));

    printPesanan(pesananQueue);

    batalkanPesanan(pesananQueue, "ORD002");
    batalkanPesanan(pesananQueue, "ORD004");

    printPesanan(pesananQueue);

    undoPembatalan(pesananQueue); 
    printPesanan(pesananQueue);

    cout << "nambah 2 pesanan baru" << endl;
    pesananQueue.push_back(Pesanan("ORD999", "2025-03-07", "P001"));
    pesananQueue.push_back(Pesanan("ORD099", "2025-03-05", "P002"));

    printPesanan(pesananQueue);

    undoPembatalan(pesananQueue); 
    printPesanan(pesananQueue);

    undoPembatalan(pesananQueue); 
    printPesanan(pesananQueue);

    return 0;
}
