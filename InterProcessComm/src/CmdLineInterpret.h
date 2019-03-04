

#ifndef COMMAND_LINE_INTERPRET_H
#define COMMAND_LINE_INTERPRET_H

#include <vector>
#include <string>

class CmdLineInterpret
{
public:
    typedef struct CommandTable
    {
        char* command;
        struct CommandTable* subCommand;
        void (*Excute)(const char* arg_);
    } COMMAND_TABLE;

public:
    void SetCmdTable(COMMAND_TABLE* pTbl_) { m_pCommandTable = pTbl_; }
    int Interpret(const char* expression_);
    int ExecutCommand(const char* arg_, COMMAND_TABLE* commandTable);

    int FindTheFistCommand(const char* expression_, size_t* pStartPost, size_t* pLength);
    int IsSeparator(const char character_, const char* separator_, int nLength_);
    char* ToUpper(const char* in_, char* out_, size_t nLength_);
    char* ToLower(const char* in_, char* out_, size_t nLength_);
    void GetAllCommand(const char* arg_, std::vector<std::string>& commands);

private:
    COMMAND_TABLE* m_pCommandTable;
};

#endif


