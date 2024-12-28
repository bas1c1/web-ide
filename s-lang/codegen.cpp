#include "codegen.h"

CodeGen::CodeGen()
{

}

void CodeGen::create_project()
{
    system("dotnet new console -n SLangApp");
    system("cd SLangApp & del Program.cs & dotnet add package Newtonsoft.Json");
    system("copy Program.cs \"SLangApp/Program.cs\"");
    system("copy Lib.cs SLangApp/Lib.cs");
}

void CodeGen::generate(std::vector<Token> tokens)
{
    fout.open("SLangApp/Program.cs", std::ios_base::app);
    fout << "void HandleGET(Socket clientSocket, string source, string path) {\n"
         << "switch (source) {\n";
    this->tokens = tokens;
    bool isErrorHandlerActive = false;
    for (size_t i = 0; i < tokens.size(); i++) {
        Token t = tokens[i];
        switch (t.tt) {
        case TokenType::WORD:
            {
                Token nt = tokens[i+1];
                if (nt.tt == TokenType::STRING && t.desc == std::string("route"))
                {
                    fout << "case \"" << nt.desc << "\": {\n";
                    i += parse_body(i + 2);
                }
                else if (nt.tt == TokenType::LBRACE && t.desc == std::string("errhandl"))
                {
                    fout << "default: {\n";
                    i += parse_body(i + 1);
                    isErrorHandlerActive = true;
                }
            }
        }
    }
    if (!isErrorHandlerActive) {
        fout << "default: {\n"
             << "   clientSocket.Send(HttpKit.HttpRespError(\"404\", \"error\"));\n"
             << "   break;\n"
             << "}\n";
    }
    fout << "}\n}";
    fout.close();
}

int CodeGen::parse_body(int offset)
{
    int tmpoffset = 1;
    if (tokens[offset].tt == TokenType::LBRACE) {
        while (tokens[offset + tmpoffset].tt != TokenType::RBRACE) {
            tmpoffset += parse_statement(offset + tmpoffset) + 1;
        }
    }
    fout << "break;\n}\n";
    return tmpoffset;
}

int CodeGen::parse_statement(int offset) {
    int tmpoffset = 0;
    //parse function
    if (tokens[offset + tmpoffset].tt == TokenType::WORD && tokens[offset + tmpoffset + 1].tt == TokenType::LBRACEC) {
        std::string fname = tokens[offset + tmpoffset].desc;
        tmpoffset += 2;
        std::vector<Token> args = std::vector<Token>();
        while (tokens[offset + tmpoffset].tt != TokenType::RBRACEC) {
            args.push_back(tokens[offset + tmpoffset]);
            tmpoffset += 1;
        }
        parse_function(fname, tmpoffset, args);
    }
    return tmpoffset;
}

void CodeGen::parse_function(std::string fname, int offset, std::vector<Token> args) {
    std::string build = parse_function_args(args);
    if (fname == "log")
    {
        fout << "Console.WriteLine(" << build << ");\n";
    }
    else if (fname == "httpResp")
    {
        fout << "clientSocket.Send(HttpKit.HttpResp(" << build << "));\n";
    }
    else if (fname == "httpRespErr")
    {
        fout << "clientSocket.Send(HttpKit.HttpRespError(" << build << "));\n";
    }
}

std::string CodeGen::parse_function_args(std::vector<Token> args)
{
    std::string build = "";
    for (int i = 0; i < args.size(); i++) {
        Token t = args[i];
        switch (t.tt) {
        case TokenType::STRING:
            {
                build += "\"" + t.desc + "\"";
                break;
            }
        default:
            {
                build += t.desc;
                break;
            }
        }
        if (i != args.size() - 1)
            build += ",";
    }
    return build;
}

void CodeGen::build()
{
    utils::crossplatform_exec("@echo off\ncd SLangApp\ndotnet build\ncls\ndotnet run");
}
