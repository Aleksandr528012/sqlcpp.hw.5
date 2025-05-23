#include <iostream>
#include <string>
#include <vector>
#include <pqxx/pqxx>


class ClientManager {
private:
    std::string connection_string;

public:
    ClientManager(const std::string& conn_str) : connection_string(conn_str) {}

    void create_tables() {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);  
    }

    int add_client(const std::string& first_name, const std::string& last_name, const std::string& email) {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);   
    }

    void add_phone(int client_id, const std::string& phone_number) {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);    
    }

    void update_client(int client_id, const std::string& first_name = "",
        const std::string& last_name = "", const std::string& email = "") {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);   
    }

    void delete_phone(const std::string& phone_number) {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);
    }

    void delete_client(int client_id) {
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);

    }
    
    std::vector<ClientManager> find_client(
        const std::string& first_name = "", const std::string& last_name = "",
        const std::string& email = "", const std::string& phone_number = ""
        )
    {
        std::vector<ClientManager> clients;
        pqxx::connection conn(connection_string);
        pqxx::work txn(conn);

        return clients;
    }
};

int main() {
    std::string conn_str = "dbname=mydb user=postgres password=12345 host=localhost port=5432";

    ClientManager manager(conn_str);

    try {
        txn.exec(
            "CREATE TABLE IF NOT EXISTS clients ("
            "client_id SERIAL PRIMARY KEY,"
            "first_name VARCHAR(50) NOT NULL,"
            "last_name VARCHAR(50) NOT NULL,"
            "email VARCHAR(100) UNIQUE NOT NULL"
            ")"
        );

        txn.exec(
            "CREATE TABLE IF NOT EXISTS phones ("
            "phone_id SERIAL PRIMARY KEY,"
            "client_id INTEGER REFERENCES clients(client_id) ON DELETE CASCADE,"
            "phone_number VARCHAR(20) UNIQUE"
            ")"
        );

        txn.commit();
        std::cout << "Tables created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating tables: " << e.what() << std::endl;
        txn.abort();
    }

    try {
        pqxx::result r = txn.exec_params(
            "INSERT INTO clients (first_name, last_name, email) VALUES ($1, $2, $3) RETURNING client_id",
            first_name, last_name, email
        );

        int client_id = r[0][0].as<int>();
        txn.commit();
        std::cout << "Client added with ID: " << client_id << std::endl;
        return client_id;
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding client: " << e.what() << std::endl;
        txn.abort();
        return -1;
    }

    try {
        txn.exec_params(
            "INSERT INTO phones (client_id, phone_number) VALUES ($1, $2)",
            client_id, phone_number
        );

        txn.commit();
        std::cout << "Phone added successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding phone: " << e.what() << std::endl;
        txn.abort();
    }

    try {
        if (!first_name.empty()) {
            txn.exec_params(
                "UPDATE clients SET first_name = $1 WHERE client_id = $2",
                first_name, client_id
            );
        }
        if (!last_name.empty()) {
            txn.exec_params(
                "UPDATE clients SET last_name = $1 WHERE client_id = $2",
                last_name, client_id
            );
        }
        if (!email.empty()) {
            txn.exec_params(
                "UPDATE clients SET email = $1 WHERE client_id = $2",
                email, client_id
            );
        }

        txn.commit();
        std::cout << "Client updated successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating client: " << e.what() << std::endl;
        txn.abort();
    }

    try {
        if (!first_name.empty()) {
            txn.exec_params(
                "UPDATE clients SET first_name = $1 WHERE client_id = $2",
                first_name, client_id
            );
        }
        if (!last_name.empty()) {
            txn.exec_params(
                "UPDATE clients SET last_name = $1 WHERE client_id = $2",
                last_name, client_id
            );
        }
        if (!email.empty()) {
            txn.exec_params(
                "UPDATE clients SET email = $1 WHERE client_id = $2",
                email, client_id
            );
        }

        txn.commit();
        std::cout << "Client updated successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating client: " << e.what() << std::endl;
        txn.abort();
    }

    try {
        txn.exec_params(
            "DELETE FROM phones WHERE phone_number = $1",
            phone_number
        );

        txn.commit();
        std::cout << "Phone deleted successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting phone: " << e.what() << std::endl;
        txn.abort();
    }

    try {
        txn.exec_params(
            "DELETE FROM clients WHERE client_id = $1",
            client_id
        );

        txn.commit();
        std::cout << "Client deleted successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting client: " << e.what() << std::endl;
        txn.abort();
    }

    try {
        std::string query =
            "SELECT c.id, c.first_name, c.last_name, c.email, p.phone_number "
            "FROM clients c "
            "LEFT JOIN client_phones p ON c.id = p.client_id "
            "WHERE 1=1";

        if (first_name) {
            query += " AND c.first_name LIKE " + txn.quote("%" + *first_name + "%");
        }
        if (last_name) {
            query += " AND c.last_name LIKE " + txn.quote("%" + *last_name + "%");
        }
        if (email) {
            query += " AND c.email LIKE " + txn.quote("%" + *email + "%");
        }
        if (phone) {
            query += " AND p.phone_number LIKE " + txn.quote("%" + *phone + "%");
        }

        query += " ORDER BY c.id";

        pqxx::result result = txn.exec(query);

        int current_id = -1;
        ClientManager current_client;

        for (auto row : result) {
            int client_id = row["id"].as<int>();

            if (client_id != current_id) {
                if (current_id != -1) {
                    clients.push_back(current_client);
                }

                current_client = ClientManager{
                    client_id,
                    row["first_name"].as<std::string>(),
                    row["last_name"].as<std::string>(),
                    row["email"].as<std::string>(),
                    {}
                };

                current_id = client_id;
            }

            if (!row["phone_number"].is_null()) {
                current_client.phones.push_back(row["phone_number"].as<std::string>());
            }
        }

        if (current_id != -1) {
            clients.push_back(current_client);
        }

        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Failed to find client: " + std::string(e.what()));
    }

    manager.create_tables();

    int client1 = manager.add_client("Иван", "Иванов", "ivan@example.com");
    int client2 = manager.add_client("Петр", "Петров", "petr@example.com");

    manager.add_phone(client1, "+79111234567");
    manager.add_phone(client1, "+79117654321");
    manager.add_phone(client2, "+79213456789");

    manager.update_client(client1, "Иван", "Иванов", "ivan.new@example.com");

    manager.find_client("Иван");
    manager.find_client("", "", "", "+79213456789");

    manager.delete_phone("+79117654321");

    manager.delete_client(client2);

    manager.find_client("Петр");

    return 0;
}