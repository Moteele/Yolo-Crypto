Server database design, in case we use client-server architecture
### ACCOUNT
**id**: unsigned int (primary key)
- can't be changed, only deleted
**login**: email / phone number, whatever we decide to be required for sign-up  
- may be changed
- unique

**displayName**: string  
- name to be shown to users regarding the account

**contacts**: unsigned int (foreign key)
- references CONTACTS table
    
**pwdHash**: string

**notifications**: CSV

### CONTACTS
**contacts_ID**: unsigned int (primary key)
**user_id**: unsigned int (foreign key - ACCOUNT)
- id of the owner
**conversations**: CSV or `foreign key array` (foreign key - CONVERSATION)
- in case of CSV format: `"user_id1, user_id2...", conversation_id, last_read_message`
    
### CONVERSATION
**conversation_id**: unsigned int (primary key)
**users**: CSV or `foreign key array` (foreign key - ACCOUNT)
**content**: CSV
* format: `time, text, "seen_by"`
    
