/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jingwu <jingwu@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 14:41:40 by hutzig            #+#    #+#             */
/*   Updated: 2025/05/09 14:46:51 by jingwu           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <iostream>
#include <unordered_set>

#include "Server.hpp"
#include "Client.hpp"
#include "Logger.hpp"

enum class USERTYPE {
    REGULAR,
    OPERATOR,
    INVITE
};

class Channel{
	public:
		Channel() = delete;
        Channel(const std::string& name, Client& user);
        Channel(const Channel&) = delete;
		Channel& operator=(const Channel& other) = delete;
		~Channel();

        const std::string& getName() const;
        const std::string& getPassword() const;
        const std::string& getTopic() const;
        bool        getInviteMode() const;
        bool        getTopicMode() const;
        bool        getPasswdMode() const;
        bool        getLimitMode() const;
        size_t      channelSize();
        const std::unordered_set<Client*>&  getChannelUsers() const;

        // Channel's set mode:
        void        setInviteOnly();
        void        unsetInviteOnly();
        void        addNewInviteUser(Client& user);

        void        setTopicRestrictions();
        void        unsetTopicRestrictions();
        void        addNewTopic(const std::string& topic);

        void        setPassword();
        void        unsetPassword();
        void        addNewPassword(const std::string& key);

        void        setLimit();
        void        unsetLimit();
        void        addLimit(int limit);

        // Regular user and Channel Operator:
        void        addNewUser(Client& user);
        void        removeUser(Client& user);

        void        addNewOperator(Client& user);
        void        removeOperator(Client& user);

        bool        isChannelUser(Client& user);
        bool        isChannelOperator(Client& user);
        bool        isInvitedUser(Client& user);

        // General:
        void        notifyChannelUsers(Client& target, std::string& msg);
        bool        isEmptyChannel();
        void        insertUser(std::shared_ptr<Client>user, USERTYPE type);

        // Channel Operators commands:
        // void        kickUser(Client& user, Client& target, const std::string& reason);
        // void        inviteUser(Client& user, Client& target);
        // void        topic(Client& user, const std::string& topic);
        // void        mode(Server& server, Client& user, const std::string& mode_flags, std::vector<std::string> args);

    private:
        std::string channel_name_;
        std::string channel_passwd_;
        std::string channel_topic_;
        bool        channel_invite_only_;
        bool        channel_restric_topic_;
        bool        channel_with_passwd_;
        bool        channel_user_limit_;
        size_t      user_limit_;

        std::unordered_set<Client*> users_; // Channels don’t own users; they just refer to them.
        std::unordered_set<Client*> operators_;
        std::unordered_set<Client*> invited_users_;

};
