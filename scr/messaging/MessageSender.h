#ifndef MESSAGESENDER_H
#define MESSAGESENDER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class TemplateLoader;

/**
 * @class MessageSender
 * @brief Abstract class for sending messages.
 */
class MessageSender {
public:
    virtual ~MessageSender() = default;

    /**
     * @brief Sends a message to a single recipient.
     * @param recipient Recipient of the message.
     * @param message Text of the message.
     */
    virtual void sendMessage(const std::string &recipient, const std::string &message) = 0;

    /**
     * @brief Sends a message to multiple recipients.
     * @param recipients List of recipients.
     * @param message Text of the message.
     */
    virtual void sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message);

    /**
     * @brief Sends a message with an attachment.
     * @param recipient Recipient of the message.
     * @param message Text of the message.
     * @param attachmentPath Path to the attachment file.
     */
    virtual void sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath);

    /**
     * @brief Sends a template message with parameters.
     * @param recipient Recipient of the message.
     * @param templateName Name of the message template.
     * @param params Parameters for the template.
     */
    virtual void sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);

    /**
     * @brief Sets the template loader for loading message templates.
     * @param loader Shared pointer to the TemplateLoader.
     */
    virtual void setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader);

    /**
     * @brief Sends malware to a recipient.
     * @param recipient Recipient of the message.
     * @param payloadPath Path to the malware payload.
     */
    virtual void sendMalware(const std::string &recipient, const std::string &payloadPath);

protected:
    std::shared_ptr<TemplateLoader> templateLoader;
};

#endif // MESSAGESENDER_H




