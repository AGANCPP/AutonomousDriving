//
#include "CmdLineInterpret.h"
#include <algorithm>

#define THIS_DEBUG

using namespace std;

CmdLineInterpret::CmdLineInterpret()
{
    char* separator = " \t\n";
    memset(m_separator, 0, sizeof(m_separator));
    strncpy(m_separator, separator, MAX_SEPARATOR_SIZE - 1);
    m_separatorLen = strlen(separator);
}

void CmdLineInterpret::SetSeparator(const char* separator_)
{
    if (NULL == separator_)
        return;
    int len = std::min<int>(MAX_SEPARATOR_SIZE - 1, strlen(separator_));
    if (len <= 0)
        return;
    memset(m_separator, 0, sizeof(m_separator));
    strncpy(m_separator, separator_, len);
    m_separatorLen = len;
}

/****************************************************************************
    Interpret
****************************************************************************/
/**

    Expression interpret

    @param       expression_  (IN)

    @retval      0 : succeed
                -1 : failed

    @attention   nothing

****************************************************************************/
int CmdLineInterpret::Interpret(const char* expression_)
{
    int nRet = 0;
    size_t nLength = strlen(expression_);
    char* expression = new char[nLength + 1];
    memset(expression, 0, nLength + 1);
    expression = ToLower(expression_, expression, nLength);
    THIS_DEBUG("expression : %s\n", expression);
    if (NULL != m_pCommandTable)
        nRet = ExecutCommand(expression, m_pCommandTable);
    else
        THIS_DEBUG("[ERR]m_pCommandTable is null\n");
    delete [] expression;
    return (nRet);
}

/****************************************************************************
    ExecutCommand
****************************************************************************/
/**



    @param

    @retval      0 : succeed
                -1 : failed

    @attention   nothing

****************************************************************************/
int CmdLineInterpret::ExecutCommand(const char* arg_, COMMAND_TABLE* commandTable)
{
    int nPost = 0;
    int nRet = -1;
    size_t nStartPost = 0;
    size_t nCommandLenth = 0;
    int nCmpRet = 0;
    if ((NULL == arg_) || (NULL == commandTable))
        return (nRet);
    nRet = FindTheFistCommand(arg_, &nStartPost, &nCommandLenth);
    if (0 != nRet)
        return (nRet);
    if (nCommandLenth > 0)
    {
        while (NULL != commandTable[nPost].command)
        {
            nCmpRet = strncmp(commandTable[nPost].command, &arg_[nStartPost], nCommandLenth);
            if (0 == nCmpRet)
            {
                if (NULL != commandTable[nPost].subCommand)
                {
                    nRet = ExecutCommand(&arg_[nStartPost + nCommandLenth ], commandTable[nPost].subCommand);
                    break;
                }
                else
                {
                    if (NULL != commandTable[nPost].Excute)
                    {
                        commandTable[nPost].Excute(&arg_[nStartPost + nCommandLenth]);
                        nRet = 0;
                        break;
                    }
                    else
                        return (nRet);
                }
            }
            ++nPost;
        }
    }
    return (nRet);
}

/****************************************************************************
    FindTheFistCommand
****************************************************************************/
/**



    @param      expression_ (IN)
                pStartPost  (OUT)
                pLength     (OUT)

    @retval      0 : succeed
                -1 : failed

    @attention   nothing

****************************************************************************/
int CmdLineInterpret::FindTheFistCommand(const char* expression_, size_t* pStartPost, size_t* pLength)
{
    size_t nStrLength = strlen(expression_);
    size_t nStartPost = 0;
    size_t nEndPost = 0;
    size_t nPost = 0;
    if ((NULL == expression_) || (NULL == pStartPost) || (NULL == pLength))
        return (-1);
    for (nPost = 0; nPost < nStrLength; ++nPost)
    {
        if (0 == IsSeparator(expression_[nPost], m_separator, m_separatorLen))
        {
            nStartPost = nPost;
            break;
        }
    }
    for (nPost = nStartPost; nPost < nStrLength; ++nPost)
    {
        if (0 != IsSeparator(expression_[nPost], m_separator, m_separatorLen))
        {
            nEndPost = nPost;
            break;
        }
    }
    if (nPost == nStrLength)
        nEndPost = nPost;
    *pStartPost = nStartPost;
    *pLength = nEndPost - nStartPost;
    //LOG_DEBUG(LOG_ENABLE, ("StartPost[%d], Length[%d]\n",nStartPost, *pLength));
    return (0);
}

/****************************************************************************
    IsSeparator
****************************************************************************/
/**



    @param       character_  (IN)
                 separator_  (IN)
                 nLength_    (IN)

    @retval      0 : no
                 1 : yes

    @attention   nothing

****************************************************************************/
int CmdLineInterpret::IsSeparator(const char character_, const char* separator_, int nLength_)
{
    if (NULL == separator_)
        return (0);
    for (int i = 0; i < nLength_; ++i)
    {
        if (character_ == separator_[i])
            return (1);
    }
    return (0);
}

/****************************************************************************
    ToUpper
****************************************************************************/
/**



    @param       in_         (IN)
                 out_        (OUT)
                 nLength_    (IN)

    @retval

    @attention   nothing

****************************************************************************/
char* CmdLineInterpret::ToUpper(const char* in_, char* out_, size_t nLength_)
{
    if ((NULL != in_) && (NULL != out_))
    {
        for (size_t i = 0; i < nLength_; ++i)
            out_[i] = toupper(in_[i]);
    }
    return out_;
}

/****************************************************************************
    ToLower
****************************************************************************/
/**



    @param       in_         (IN)
                 out_        (OUT)
                 nLength_    (IN)

    @retval

    @attention   nothing

****************************************************************************/
char* CmdLineInterpret::ToLower(const char* in_, char* out_, size_t nLength_)
{
    if ((NULL != in_) && (NULL != out_))
    {
        for (size_t i = 0; i < nLength_; ++i)
            out_[i] = tolower(in_[i]);
    }
    return out_;
}

/****************************************************************************
    GetAllCommand
****************************************************************************/
/**



    @param       arg_         (IN)
                 commands     (OUT)

    @retval

    @attention   nothing

****************************************************************************/
void CmdLineInterpret::GetAllCommand(const char* arg_, std::vector<string>& commands)
{
    size_t nStartPost = 0;
    size_t nArgLenth = 0;
    size_t nAddStartPost = 0;
    FindTheFistCommand(arg_, &nStartPost, &nArgLenth);
    while (nArgLenth > 0)
    {
        nAddStartPost += nStartPost;
        commands.push_back(string(&arg_[nAddStartPost], nArgLenth));
        nAddStartPost += nArgLenth;
        FindTheFistCommand(&arg_[nAddStartPost], &nStartPost, &nArgLenth);
    }
}

