#include <memory>
#include <vector>
#include <string>
#include <ctime>
#include <random>

template <typename T>
void delete_vector( std::vector<T*>* vector )
{
    // avoid unsigned underflow
    for ( int32_t i = ( int32_t ) vector->size() - 1; i >= 0; --i )
    {
        auto element = vector->at( i );
        vector->erase( vector->begin() + i );
        delete element;
    }
    vector->clear();
    vector->shrink_to_fit();
}

class Transaction
{
public:
    Transaction() {};
    Transaction( std::string sender, std::string receiver, double amount ) :
        m_sender( sender ), m_receiver( receiver ), m_amount( amount )
    {};
    ~Transaction() {}

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
        m_index = ( ++g_index_count );
        m_timestamp = std::time( 0 );
    }

    ~Block()
    {
        delete_vector( &m_transactions );
    }

    std::string hash()
    {
        // temporary hash function
        return std::to_string( m_timestamp ) + "__block";
    }

    void new_transaction( std::string sender, std::string receiver, double amount )
    {
        m_transactions.push_back( new Transaction( sender, receiver, amount ) );
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
        Block* genesis_block = new Block( "1", 100 );
        m_block_chain.push_back( genesis_block );

        m_current_block = new Block( genesis_block->hash(), 100 );
    }

    ~BlockChain()
    {
        delete_vector( &m_block_chain );
    }

    void new_block()
    {
        std::string previous_hash = m_current_block->hash();
        m_block_chain.push_back( m_current_block );

        m_current_block = nullptr;
        m_current_block = new Block( previous_hash, 100 );
    }

    void new_transaction( std::string sender, std::string receiver, double amount )
    {
        m_current_block->new_transaction( sender, receiver, amount );
    }

private:
    Block* m_current_block = nullptr;
    std::vector<Block*> m_block_chain;
};

int main( int argc, const char* argv[] )
{

    BlockChain* tmpcoin = new BlockChain();

    std::default_random_engine random_engine;
    std::uniform_int_distribution<int32_t> addr_range( 100000000, 999999999 );
    std::uniform_real_distribution<double> amount_range( 1.0, 100.0 );

    for ( int32_t b = 0; b < 10; ++b )
    {
        for ( int32_t t = 0; t < 20; ++t )
        {
            tmpcoin->new_transaction(
                std::to_string( addr_range( random_engine ) ),
                std::to_string( addr_range( random_engine ) ),
                amount_range( random_engine ) );
        }

        tmpcoin->new_block();
    }

    delete tmpcoin;

    return 0;
}