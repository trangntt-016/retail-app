#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <cstring>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;

using namespace oracle::occi;

struct ShoppingCart
{
	int product_id;
	double price;
	int quantity;
};

//prototype of the functions
int mainMenu();
int getInt();
int customerLogin(Connection*, int);
int addToCart(Connection*, struct ShoppingCart cart[]);
double findProduct(Connection*, int);
void displayProducts(struct ShoppingCart cart[], int);
int checkout(Connection*, struct ShoppingCart cart[], int, int);
char getYN();

int main(void) {
	// declare struct ShoppingCart variables
	ShoppingCart *cart = new ShoppingCart[5];

	//OCCI variables
	Environment* env = nullptr;
	Connection* conn = nullptr;

	//User variables
	std::string str;
	std::string usr = "dbs311_203e28";
	std::string pass = "11253449";
	std::string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";

	//create Environment and connection
	env = Environment::createEnvironment(Environment::DEFAULT);
	conn = env->createConnection(usr, pass, srv);

	// main part of the program
	bool isLogOut = false;
	while (!isLogOut) {
		int found = 0, custID = 0;
		int option = mainMenu();
		int noOfAddedItems = 0;
		int ischeckedOut = 0;
		switch (option) {
		case 1:
			// prompt the user to enter customer ID and make sure it is of right type (integer)
			std::cout << "Enter the customer ID :";
			custID = getInt();
			try {
				found = customerLogin(conn, custID);
			}
			catch (SQLException& sqlExcp) {
				std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
			}
			if (found == 0) {
				std::cout << "The customer does not exist.\n";
			}
			else if(found == 1){
				try {
					noOfAddedItems = addToCart(conn, cart);
				}
				catch (SQLException& sqlExcp) {
					std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
				}
				displayProducts(cart, noOfAddedItems);
				ischeckedOut = checkout(conn, cart, custID, noOfAddedItems);
				switch (ischeckedOut) {
				case 0:
					std::cout << "The order is cancelled.\n";
					break;
				case 1:
					std::cout << "The order is successfully completed.\n";
					break;
				}
			}
			break;
		case 0:
			std::cout << "Good bye!...";
			isLogOut = true;
			break;
		}
	}

	// terminate Environment and Connection
	env->terminateConnection(conn);
	Environment::terminateEnvironment(env);
	return 0;

}

int mainMenu() {
	int option;
	std::cout << "************* Main Menu by Trang Nguyen - 131104192 ********************" << std::endl;
	std::cout << "1)      Login" << std::endl;
	std::cout << "0)      Exit" << std::endl;
	std::cout << "Enter an option (0-1):";
	std::cin >> option;
	while (option != 0 && option != 1 || !std::cin.good()) {
		// if option is of type string or character, flush the buffer so that it wont have infinite loop
		if (!std::cin.good()) {
			std::cin.clear();
			std::cin.ignore(2000, '\n');
		}
		std::cout << "************* Main Menu by Trang Nguyen - 131104192 ********************" << std::endl;
		std::cout << "1)      Login" << std::endl;
		std::cout << "0)      Exit" << std::endl;
		std::cout << "You entered a wrong value. Enter an option (0-1):";
		std::cin >> option;
	}
	return option;
}

// validate the ID
int getInt() {
	int custID;
	std::cin >> custID;
	while (!std::cin.good()) {
		std::cin.clear();
		std::cin.ignore(2000, '\n');
		std::cout << "Please enter a number: ";
		std::cin >> custID;
	}
	return custID;
}
// execute query to find customer
int customerLogin(Connection* conn,int custID) {
	int found = 0;
	Statement* stmt = conn->createStatement();
	stmt = conn->createStatement();
	// calll the stored procedure
	stmt->setSQL("BEGIN find_customer(:1, :2); END;");
	// the input paramenter receives custID
	stmt->setInt(1, custID);
	// set the output parameter
	stmt->registerOutParam(2, Type::OCCIINT, sizeof(found));
	stmt->executeUpdate();
	// get the output value
	found = stmt->getInt(2);
	//terminate statement
	conn->terminateStatement(stmt);
	return found;
}

double findProduct(Connection* conn, int product_id) {
	double price = 0.0;
	Statement* stmt = conn->createStatement();
	stmt = conn->createStatement();
	// calll the stored procedure
	stmt->setSQL("BEGIN find_product(:1, :2); END;");
	// the input paramenter receives custID
	stmt->setInt(1, product_id);
	// set the output parameter
	stmt->registerOutParam(2, Type::OCCIDOUBLE, sizeof(price));
	stmt->executeUpdate();
	// get the output value
	price = stmt->getDouble(2);
	//terminate statement
	conn->terminateStatement(stmt);

}

int addToCart(Connection* conn, struct ShoppingCart cart[]) {
	int noOfItems = 0, prodID = 0,quantity = 0;
	double price = 0;
	int isExit = 1;
	std::cout << "-------------- Add Products to Cart --------------" << std::endl;
	while (noOfItems < 5 && isExit==1) {
		std::cout << "Enter the product ID:";
		prodID = getInt();
		price = findProduct(conn,prodID);
		if (price == 0){
			std::cout << "The product does not exists. Try again..."<<std::endl;
		}
		else {
			std::cout << "Product Price:" << price<<std::endl;
			std::cout << "Enter the product Quantity:";
			quantity = getInt();
			
			// store the current cart
			cart[noOfItems].price = price;
			cart[noOfItems].product_id = prodID;
			cart[noOfItems].quantity = quantity;

			std::cout << "Enter 1 to add more products or 0 to checkout:";
			isExit = getInt();
			if (isExit == 1) {
				++noOfItems;
			}
		}
	}
	return noOfItems + 1;
};

void displayProducts(struct ShoppingCart cart[], int productCount) {
	double total = 0;
	std::cout << "------- Ordered Products ---------" << std::endl;
	for (int i = 0; i < productCount; i++) {
		std::cout << "---Item "<<i + 1<<std::endl;
		std::cout << "Product ID: " << cart[i].product_id<<std::endl;
		std::cout << "Price: " << cart[i].price<<std::endl;
		std::cout << "Quantity: " << cart[i].quantity<<std::endl;
		total += cart[i].price * cart[i].quantity;
	}
	std::cout << "----------------------------------\n";
	std::cout << "Total: "<<total<<std::endl;
};

char getYN() {
	char yNo;
	std::cin >> yNo;
	while (yNo != 'y' && yNo != 'Y' && yNo != 'n' && yNo != 'N') {
		std::cout << "Wrong input. Try again...\n";
		std::cin.clear();
		std::cin.ignore(2000, '\n');
		std::cout << "Would you like to checkout ? (Y / y or N / n) ";
		std::cin >> yNo;
	}
	return yNo;
}
int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount) {
	int isCheckedOut = 0;
	char yNo;
	std::cout<<"Would you like to checkout ? (Y / y or N / n) ";
	yNo = getYN();
	if (yNo == 'y' || yNo == 'Y') {
		int newOrderID;
		Statement* stmt = conn->createStatement();
		stmt = conn->createStatement();
		// calll the stored procedure
		stmt->setSQL("BEGIN add_order(:1, :2); END;");
		// the input paramenter receives custID
		stmt->setInt(1, customerId);
		// set the output parameter
		stmt->registerOutParam(2, Type::OCCIINT, sizeof(newOrderID));
		stmt->executeUpdate();
		// get the new ID
		newOrderID = stmt->getInt(2);
		// add the items to the cart
		for (int i = 0; i < productCount; i++) {
			stmt->setSQL("BEGIN add_order_item(:1,:2,:3,:4,:5); END;");
			// the 1st receives orderID 
			stmt->setInt(1, newOrderID);
			// the 2nd receives itemID 
			stmt->setInt(2, i+1);
			// the 3rd receives productID 
			stmt->setInt(3, cart[i].product_id);
			// the 4th receives quantity 
			stmt->setInt(4, cart[i].quantity);
			// the 5th receives price 
			stmt->setDouble(5, cart[i].price);
			stmt->executeUpdate();
		}
		//terminate statement
		conn->terminateStatement(stmt);
		isCheckedOut = 1;
	}
	return isCheckedOut;
};