/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jingwu <jingwu@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 09:15:08 by arissane          #+#    #+#             */
/*   Updated: 2025/05/20 14:56:11 by arissane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include "Logger.hpp"
#include <string>
#include <sstream>

Message::Message(std::string& message) :
      whole_msg_(message),
      number_of_parameters_(0),
      msg_trailing_(""),
      parameters_(),
      msg_users_(),
      msg_channels_(),
      cmd_type_(INVALID),
      cmd_string_(""),
      passwords_(),
      msg_trailing_empty_(false)
{
    initCommandHandlers();
}

Message::~Message(){
}

void Message::initCommandHandlers(){
    command_handlers_ = {
        {"PASS",   [this](){ cmd_type_ = PASS; return handlePASS(); }},
        {"NICK",   [this](){ cmd_type_ = NICK; return handleGeneric(); }},
        {"TOPIC",  [this](){ cmd_type_ = TOPIC; return handleGeneric(); }},
        {"USER",   [this](){ cmd_type_ = USER; return handleGeneric(); }},
        {"PRIVMSG",[this](){ cmd_type_ = PRIVMSG; return handleGeneric(); }},
        {"PART",   [this](){ cmd_type_ = PART; return handleGeneric(); }},
        {"JOIN",   [this](){ cmd_type_ = JOIN; return handleJOIN(); }},
        {"QUIT",   [this](){ cmd_type_ = QUIT; return handleGeneric(); }},
        {"INVITE", [this](){ cmd_type_ = INVITE; return handleGeneric(); }},
        {"MODE",   [this](){ cmd_type_ = MODE; return handleMODE(); }},
        {"CAP",   [this](){ cmd_type_ = CAP; return handleCAP(); }},
        {"PING",   [this](){ cmd_type_ = PING; return handleNoParse(); }},
        {"WHOIS",   [this](){ cmd_type_ = WHOIS; return handleGeneric(); }},
        {"WHO",   [this](){ cmd_type_ = WHO; return handleNoParse(); }},
        {"KICK",   [this](){ cmd_type_ = KICK; return handleKICK(); }}
    };
}

bool Message::handleNoParse(){
    return true;
}

bool Message::handlePASS(){
    for (size_t i = 0; i < parameters_.size(); ++i){
        if (parameters_[i][0] == '#') {
            msg_channels_.push_back(parameters_[i]);
        } else{
            passwords_.push_back(parameters_[i]);
        }
    }
    return true;
}

bool Message::handleCAP(){
    if (parameters_.empty()){
        Logger::log(Logger::ERROR, "CAP command missing subcommand");
        return false;
    }
    return true;
}

bool Message::handleGeneric(){
    for (size_t i = 0; i < parameters_.size(); ++i){
        if (parameters_[i][0] == '#') {
            msg_channels_.push_back(parameters_[i]);
        } else{
            msg_users_.push_back(parameters_[i]);
        }
    }
    return true;
}

bool Message::handleJOIN(){
    for (size_t i = 0; i < parameters_.size(); ++i){
        if (parameters_[i][0] == '#') {
            msg_channels_.push_back(parameters_[i]);
        } else{
            passwords_.push_back(parameters_[i]);
        }
    }
    return true;
}

bool Message::handleKICK(){
    if (parameters_.size() < 2){
        Logger::log(Logger::ERROR, "KICK should contain at least 2 parameters");
	    return false;
    }
    if (parameters_[0][0] == '#'){
        msg_channels_.push_back(parameters_[0]);
    } else{
        Logger::log(Logger::ERROR, "KICK first parameter should be a channel");
	    return false;
    }
    if (parameters_[1][0] == '#'){
        Logger::log(Logger::ERROR, "KICK second parameter should not be a channel");
	    return false;
    } else{
        msg_users_.push_back(parameters_[1]);
    }
    for (size_t i = 2; i < parameters_.size(); ++i){
        if (parameters_[i][0] == '#'){
            msg_channels_.push_back(parameters_[i]);
        } else{
            msg_users_.push_back(parameters_[i]);
        }
    }
    return true;
}

bool Message::handleMODE(){
    if (parameters_.empty()){
        Logger::log(Logger::ERROR, "MODE should contain parameters");
        return false;
    }
    if (parameters_[0][0] == '#'){
        msg_channels_.push_back(parameters_[0]);
    } else{
        msg_users_.push_back(parameters_[0]);
    }
    for (size_t i = 1; i < parameters_.size(); ++i){
        if (parameters_[i][0] == '#') {
            msg_channels_.push_back(parameters_[i]);
        } else{
            msg_users_.push_back(parameters_[i]);
        }
    }
    return true;
}

bool Message::validateParameters(const std::string& command){
    auto it = command_handlers_.find(command);
    if (it != command_handlers_.end()){
        return it->second();
    }
    Logger::log(Logger::ERROR, "Unknown command: " + command);
    return false;
}

/**
 * Transforms the entire message received into a istringstream and extracts
 * one by one all the parts of it separated by spaces, then further separates each
 * word by commas if any are found. The command is saved in thecmd_string_ and the
 * trailing message in msg_trailing, everything else is pushed to the parameters_ vector
 * and then validated separately for each command.
 */
bool Message::parseMessage(){
    std::istringstream input_stream(whole_msg_);
    std::string command;
    std::string     word;

    input_stream >> command;
    cmd_string_ = command;
    while (input_stream >> word){
        if (!word.empty() && word[0] == ':'){
            std::string rest_of_line;
            std::getline(input_stream, rest_of_line);
            // trim the end '\n' and '\r' out
            while (rest_of_line.back() == '\n' || rest_of_line.back() == '\r'){
                rest_of_line.pop_back();
            }
            if (!rest_of_line.empty()){
            msg_trailing_ = word.substr(1) + rest_of_line;
            }
            else{
            msg_trailing_ = word.substr(1);
            if (msg_trailing_.empty()){
                msg_trailing_empty_ = true;
            }
            }
            break;
        }
        // trim the end '\n' and '\r' out
        while (word.back() == '\n' || word.back() == '\r'){
            word.pop_back();
        }
        std::stringstream string_stream(word);
        std::string token;
        while (std::getline(string_stream, token, ',')){
            if (!token.empty()){
                ++number_of_parameters_;
                parameters_.push_back(token);
            }
        }
    }
    if (validateParameters(command) == false){
        Logger::log(Logger::ERROR, "Validation failed");
        return false;
    }
    return true;
}

const std::string& Message::getWholeMessage() const{
    return whole_msg_;
}

int Message::getNumberOfParameters() const{
    return number_of_parameters_;
}

const std::string& Message::getTrailing() const{
    return msg_trailing_;
}

const std::vector<std::string>& Message::getParameters() const{
    return parameters_;
}

const std::vector<std::string>& Message::getUsers() const{
    return msg_users_;
}

const std::vector<std::string>& Message::getChannels() const{
    return msg_channels_;
}

COMMANDTYPE Message::getCommandType() const{
    return cmd_type_;
}

const std::string& Message::getCommandString() const{
    return cmd_string_;
}

const std::vector<std::string>& Message::getPasswords() const{
    return passwords_;
}

bool Message::getTrailingEmpty() const{
    return msg_trailing_empty_;
}

/*
// for testing only
void	Message::printMsgInfo() const{
    std::cout << "Message Info:\n";
    std::cout << "  whole_msg=" << whole_msg_ << std::endl;
    // std::cout << "  msg_trailing=" << msg_trailing_ << std::endl;
    // std::cout << "  cmd_string_=" << cmd_string_ << std::endl;
    // std::cout << "  number_of_parameters_=" << std::to_string(number_of_parameters_) << std::endl;
    // std::cout << "  number_of_user=" << msg_users_.size() << std::endl;
    // std::cout << "  number_of_channel=" << msg_channels_.size() << std::endl;
    // std::cout << "  number_of_password=" << passwords_.size() << std::endl;
    // if (msg_users_.size() > 0){
    //     printUserList();
    // }
}

void	Message::printUserList() const{
    std::cout<< "Users in a Message class:";
    for (auto& name : msg_users_){
        std::cout << name << ", ";
    }
    std::cout << std::endl;
}*/
