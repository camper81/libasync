#pragma once

void flush(std::ostream& os, std::vector<std::string>& vec) {
    os << "bulk: ";

    for(auto it = vec.begin(), end = vec.end(); it != prev(end); ++it){
        os << *it << ", ";
    }
    os << *prev(vec.end()) << std::endl;
}

void flushCommands(time_t first_cmd, std::vector<std::string>& vec){
    if(vec.empty())
        return;

    std::string filename = "bulk" + std::to_string(first_cmd) + ".log";
    std::ofstream ofs(filename,std::ios_base::in | std::ios_base::app);

    flush(ofs, vec);
    ofs.close();

    flush(std::cout, vec);

    vec.clear();
}

class Parser;

class State
{
public:
    virtual void getCommand(Parser *parser, std::string& cmd) = 0;

    virtual void openBracket(Parser *parser) = 0;

    virtual void closeBracket(Parser *parser) = 0;

    virtual void endFile(Parser *parser) = 0;
};

class BulkState : public State
{
    int n_blocks_;
    std::vector<std::string> cmds_;
    time_t first_cmd_ = 0;
public:
    BulkState(int n_blocks) : n_blocks_(n_blocks) {};

    void getCommand(Parser* parser, std::string& cmd) override;

    void openBracket(Parser* parser) override;

    void closeBracket(Parser* parser) override {
    }

    void endFile(Parser *parser) override;
};

class OpenBracketState : public State
{
    int level_;
    std::vector<std::string> cmds_;
    time_t first_cmd_ = 0;
public:
    void getCommand(Parser* parser, std::string& cmd) override
    {
        if(first_cmd_ == 0)
            first_cmd_ = time(NULL);
        cmds_.push_back(cmd);
    }

    void openBracket(Parser* parser) override
    {
        level_++;
    }
    void closeBracket(Parser* parser) override;

    void endFile(Parser *parser) override {
    };
};

class Parser
{
    State* state_;
    int n_blocks_ = 3;
    bool EOF_ = false;
public:

    Parser(int n_blocks) : n_blocks_(n_blocks)
    {
        state_ = new BulkState(n_blocks_);
    }
    int getNBlocks() const {
        return n_blocks_;
    }

    bool isEof() const {
        return EOF_;
    }

    void parse(std::string str){
        if(str == "{")
            openBracket();
        else if(str == "}")
            closeBracket();
        else if(str == "EOF")
            getEOF();
        else
            getCommand(str);
    }
    void set_state(State* state)
    {
        this->state_ = state;
    }

    void getCommand(std::string command) {
        state_->getCommand(this, command);
    }

    void openBracket() {
        state_->openBracket(this);
    }

    void closeBracket() {
        state_->closeBracket(this);
    }

    void getEOF(){
        EOF_ = true;
        state_->endFile(this);
    }
};

void BulkState::endFile(Parser *parser) {
    flushCommands(first_cmd_, cmds_);
}

void BulkState::getCommand(Parser* parser, std::string& cmd)
{
    if(first_cmd_ == 0)
        first_cmd_ = time(NULL);
    cmds_.push_back(cmd);

    if(--n_blocks_ == 0) {
        flushCommands(first_cmd_, cmds_);
        n_blocks_ = parser->getNBlocks();
        first_cmd_ = 0;
    }
}

void OpenBracketState::closeBracket(Parser *parser) {
    if(level_-- == 0){
        flushCommands(first_cmd_, cmds_);
        parser->set_state(new BulkState(parser->getNBlocks()));
        delete this;
    }
}


void BulkState::openBracket(Parser *parser) {
    flushCommands(first_cmd_, cmds_);
    parser->set_state(new OpenBracketState());
    delete this;
}