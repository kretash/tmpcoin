#include <memory>
#include <vector>
#include <string>
#include <ctime>
#include <random>
#include <iostream>
#include <iomanip>

#include "picosha2.h"
#include "main.h"

// HELPER FUNCTIONS ------------------------------------------
namespace helper
{
    template <typename T>
    void delete_vector( std::vector<T>* vector )
    {
        for ( size_t i = 0; i < vector->size(); ++i )
        {
            auto elem = vector->at( i );
            delete elem;
        }
        vector->clear();
        vector->shrink_to_fit();
    }

    std::string hash( std::string value )
    {
        std::vector<unsigned char> hash( 32 );
        picosha2::hash256( value.begin(), value.end(), hash.begin(), hash.end() );
        std::string hex_str = picosha2::bytes_to_hex_string( hash.begin(), hash.end() );
        return hex_str;
    }
}
// END OF HELPER FUNCTIONS ------------------------------------------

class Transaction
{
public:
    Transaction() {};
    Transaction( std::string sender, std::string receiver, double amount ) :
        m_sender( sender ), m_receiver( receiver ), m_amount( amount )
    {};
    ~Transaction() {}

    double check_balance( std::string public_key )
    {
        if ( m_sender == public_key )
        {
            return -m_amount;
        }
        else if ( m_receiver == public_key )
        {
            return m_amount;
        }
        return 0.0;
    }

private:
    std::string m_sender = "";
    std::string m_receiver = "";
    double m_amount = 0.0;
};

int64_t g_index_count = 0;
class Block
{
public:
    Block( std::string previous_hash, int64_t proof )
    {
        m_previous_hash = previous_hash;
        m_proof = proof;

        // start at 1.
        m_index = (++g_index_count);
        m_timestamp = std::time( 0 );
    }

    ~Block()
    {
        helper::delete_vector( &m_transactions );
    }

    std::string hash()
    {
        std::string to_hash =
            std::to_string( m_index ) +
            std::to_string( m_timestamp ) +
            std::to_string( m_proof ) +
            std::to_string( m_transactions.size() );

        return helper::hash( to_hash );
    }

    void new_transaction( std::string sender, std::string receiver, double amount )
    {
        m_transactions.push_back( new Transaction( sender, receiver, amount ) );
    }

    double check_balance( std::string public_key )
    {
        double amount = 0.0;
        for ( Transaction* transaction : m_transactions )
        {
            double _balance = transaction->check_balance( public_key );
            amount += _balance;
        }
        return amount;
    }

    int64_t get_proof()
    {
        return m_proof;
    }

private:
    int64_t m_index = 0;
    std::time_t m_timestamp = 0;
    std::vector<Transaction*> m_transactions;
    int64_t m_proof = 0;
    std::string m_previous_hash = "";
};

class BlockChain
{
public:
    BlockChain()
    {
        Block* genesis_block = new Block( "0", 0 );
        m_current_block = genesis_block;
    }

    ~BlockChain()
    {
        helper::delete_vector( &m_block_chain );
    }

    void new_block( int64_t proof )
    {
        std::string previous_hash = m_current_block->hash();
        m_block_chain.push_back( m_current_block );

        m_current_block = nullptr;
        m_current_block = new Block( previous_hash, proof );
    }

    void new_transaction( std::string sender, std::string receiver, double amount )
    {
        m_current_block->new_transaction( sender, receiver, amount );

        std::cout << "TRANSACTION: " << sender << " -> " << receiver << " : " << std::setprecision( 8 ) << amount << "\n";
    }

    double check_balance( std::string public_key )
    {
        double amount = 0.0;
        for ( Block* block : m_block_chain )
        {
            double _balance = block->check_balance( public_key );
            amount += _balance;
        }
        return amount;
    }

    int64_t proof_of_work( int64_t last_proof )
    {
        int64_t proof = 0;

        while ( valid_proof( last_proof, proof ) == false )
        {
            proof = proof + 1;
        }

        return proof;
    }

    bool valid_proof( int64_t last_proof, int64_t proof )
    {
        std::string guess = std::to_string( last_proof ) + std::to_string( proof );
        std::string guess_hash = helper::hash( guess );
        std::string end = std::string( guess_hash.end() - 3, guess_hash.end() );
        return (end == "000");
    }

    Block* m_current_block = nullptr;
private:
    std::vector<Block*> m_block_chain;
};

class Entity
{
public:
    Entity( std::string address ) { m_public_key = address; }
    ~Entity() {}

    void send( BlockChain* chain, double amount, std::string receiver )
    {
        chain->new_transaction( m_public_key, receiver, amount );
    }

    void mine( BlockChain* chain )
    {
        Block* current_block = chain->m_current_block;
        int64_t last_proof = current_block->get_proof();
        int64_t proof = chain->proof_of_work( last_proof );

        chain->new_transaction( "0", m_public_key, 1.0 );

        chain->new_block( proof );
    }

    std::string m_public_key = "";

private:
    std::string m_private_key = "";
    double m_coins = 0.0;
};

int main( int argc, const char* argv[] )
{
    BlockChain* tmpcoin = new BlockChain();

    std::default_random_engine random_engine;
    std::uniform_int_distribution<int32_t> addr_range( 100000000, 999999999 );
    std::uniform_real_distribution<double> amount_range( 0.01, 1.0 );

    std::vector<Entity> users;
    for ( int32_t u = 0; u < 5; ++u )
    {
        users.push_back( Entity( std::to_string( addr_range( random_engine ) ) ) );
    }

    std::uniform_int_distribution<int32_t> entity_range( 0, (int32_t) (users.size()) - 1 );

    for ( int32_t b = 0; b < 4; ++b )
    {
        for ( int32_t t = 0; t < 4; ++t )
        {
            int32_t ru1 = entity_range( random_engine );
            int32_t ru2 = entity_range( random_engine );

            if ( ru1 == ru2 ) continue;

            double amount = amount_range( random_engine );
            std::string receiver = users[ru2].m_public_key;

            users[ru1].send( tmpcoin, amount, receiver );
        }

        int32_t mu = entity_range( random_engine );
        users[mu].mine( tmpcoin );

        std::cout << " NEW BLOCK ### \n";
    }

    std::cout << "\n\n\n\n";

    for ( size_t u = 0; u < users.size(); ++u )
    {
        std::string public_key = users[u].m_public_key;
        double amount = tmpcoin->check_balance( public_key );

        std::cout << "BALANCE: " << public_key << " : " << std::setprecision( 8 ) << amount << "\n";
    }

    delete tmpcoin;

    return 0;
}