#include "gtest/gtest.h"
#include "BackEnd.h"

//Write your unit tests here

// 1. A query returns all available flight bookings from a source airport to a destination airport.
TEST(BackEndTest, ReturnsAllAvailableDirectFlightsOneFlight){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(10,8,sea,lax); // 10 passengers, leaves at 8, from sea, to lax
	std::vector<Flight*> allFlights = {&f1};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),1);}

TEST(BackEndTest, ReturnsAllAvailableDirectFlightsTwoFlights){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(10,8,sea,lax); // 10 passengers, leaves at 8, from sea, to lax
	Flight f2(11,10,sea,lax);
	std::vector<Flight*> allFlights = {&f1,&f2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),2);}

// 2. A query will not return any flight booking that includes a full flight.
TEST(BackEndTest, DoesNotReturnFullBookingOnlyFullFlight){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(0,8,sea,lax); // 0 Passengerss
	std::vector<Flight*> allFlights = {&f1};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),0);}

TEST(BackEndTest, DoesNotReturnFullBookingOtherFlightOptions){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(0,8,sea,lax); // 0 Passengerss
	Flight f2(8,10,sea,lax);
	std::vector<Flight*> allFlights = {&f1,&f2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),1);}

// 3. When a flight booking is purchased, the capacities of all flights in the booking should be decremented.
TEST(BackEndTest, PurchaseDecreasesCapacityOneFlight){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(10,8,sea,lax);
	std::vector<Flight*> flights = {&f1};
	Booking myBooking(flights);

	purchaseBooking(myBooking);
	int test_result = f1.getCapacity();
	ASSERT_EQ(test_result,9);}

TEST(BackEndTest, PurchaseDecreasesCapacityMultipleFlightsF1Look){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(10,8,sea,lax);
	Flight f2(11,12,sea,lax);
	std::vector<Flight*> flights = {&f1,&f2};
	Booking myBooking(flights);

	purchaseBooking(myBooking);
	int test_result = f1.getCapacity();
	ASSERT_EQ(test_result,9);}

TEST(BackEndTest, PurchaseDecreasesCapacityMultipleFlightsF2Look){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight f1(10,8,sea,lax);
	Flight f2(11,12,sea,lax);
	std::vector<Flight*> flights = {&f1,&f2};
	Booking myBooking(flights);

	purchaseBooking(myBooking);
	int test_result = f2.getCapacity();
	ASSERT_EQ(test_result,10);}

// 4.A booking either contains a single direct flight from the source airport to the destination
//	airport, or two flights: one from the source airport to a connection airport, and a second
//	from the connection airport to the destination airport. The second flight in a connection
//	must depart at least 2 hours after the first flight departure.
TEST(BackEndTest, ConnectionUnderTwoHourGap){
	Airport sea("SEA");
	Airport sfo("SFO");
	Airport lax("LAX");
	Flight f1(10,8,sea,sfo);
	Flight f2(10,9,sfo,lax);
	std::vector<Flight*> allFlights = {&f1, &f2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),0);}

TEST(BackEndTest, ConnectionExactlyTwoHourGap){
	Airport sea("SEA");
	Airport sfo("SFO");
	Airport lax("LAX");
	Flight f1(10,8,sea,sfo);
	Flight f2(10,10,sfo,lax);
	std::vector<Flight*> allFlights = {&f1, &f2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),1);}

TEST(BackEndTest, ConnectionOverTwoHourGap){
	Airport sea("SEA");
	Airport sfo("SFO");
	Airport lax("LAX");
	Flight f1(10,8,sea,sfo);
	Flight f2(10,9,sfo,lax);
	std::vector<Flight*> allFlights = {&f1, &f2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),1);}

// 5. A query will only return bookings with connections when there are no direct flights from
//	the source airport to the destination airport.
TEST(BackEndTest, PreferDirectOverConnectionWithConnectAndDirectAsOption){
	Airport sea("SEA");
	Airport sfo("SFO");
	Airport lax("LAX");
	Flight direct(10,12,sea,lax);
	Flight layover1(10,8,sea,sfo);
	Flight layover2(10,11,sfo,lax);
	std::vector<Flight*> allFlights = {&direct, &layover1, &layover2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),1);}

TEST(BackEndTest, PreferDirectOverConnectionWithOnlyConnectAsOption){
	Airport sea("SEA");
	Airport sfo("SFO");
	Airport lax("LAX");
	Flight layover1(10,8,sea,sfo);
	Flight layover2(10,11,sfo,lax);
	std::vector<Flight*> allFlights = {&layover1, &layover2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),2);}

// 6. If multiple flight bookings are returned by the query, bookings should be ordered by
//	departure time, with the earliest flight first. In the case that a booking is a connection, it is
//	ordered solely based on the departure time of the earlier flight.
TEST(BackEndTest, BookingsAreSortedByTime){
	Airport sea("SEA");
	Airport lax("LAX");
	Flight early(10,6,sea,lax);
	Flight late(10,22,sea,lax);
	std::vector<Flight*> allFlights = {&early, &late};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	// Safety Check to prevent Segmentation Defaults
	ASSERT_FALSE(test_result.empty());

	int first_departure = test_result[0].getFlights()[0]->getTakeoffHour();
	ASSERT_EQ(first_departure, 6);}

// 7. Ensure ultiple valid direct flights are all returned.
TEST(BackEndTest, ReturnsMultipleDirectFlights){
	Airport sea("SEA");
	Airport lax("Lax");
	Flight f1(10,8,sea,lax);
	Flight f2(10,15,sea,lax);
	std::vector<Flight*> allFlights = {&f1, &f2};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),2);}

TEST(BackEndTest, ReturnsMultipleConnectionFlights){
	Airport sea("SEA");
	Airport sof("SOF");
	Airport lax("Lax");
	Flight connection11(10,8,sea,sof);
	Flight connection12(9,15,sof,lax);
	Flight connection21(11,5,sea,sof);
	Flight connection22(8,7,sof,lax);
	std::vector<Flight*> allFlights = {&connection11, &connection12, &connection21,&connection22};

	std::vector<Booking> test_result = flightBookingQuery(allFlights, sea, lax);
	ASSERT_EQ(test_result.size(),4);}
