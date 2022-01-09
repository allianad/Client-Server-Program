# Client-Server Program
## Functionality

The client successfully makes a TCP connection to the indirection server and makes UDP connections from indirection server to the micro servers. Client can request an english word from the chosen five to translate to french. Client can also use the currency conversion service. Lastly client can see the list of candidates to vote for, vote once securely with the use of an encryption key and only see the results after voting. Indirection server also notifies client if there is no response after 3 seconds from the micro servers.

## How to compile and configure

Open 5 terminal windows for each cpp file. Run the indServer.cpp and the micro servers programs in the following IP, 136.159.5.25 and the client program in 136.159.5.27.

First compile and run the indServer.cpp, type the following:
g++ indServer.cpp -o i
./i 136.159.5.25-12333

Then compile and run the client.cpp, type the following:
g++ client.cpp -o c
./c 136.159.5.25-12333

Then compile and run each of the micro server programs in separate terminals. There are no arguments needed. Type the following:
g++ translate.cpp -o t
./t

g++ vote.cpp -o v
./v

g++ convert.cpp -o con
./con

## How to use

When the client program is run, user has to select a service. To choose a service client types a one digit number in the client terminal.
0 End Session.
1 Translate english word to french.
2 Convert Canadian dollars to another currency.
3 Show candidates.
4 Vote.
5 View Voting Results.

