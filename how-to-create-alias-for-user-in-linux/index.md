# how to create alias for user in Linux


here:

<https://unix.stackexchange.com/questions/758076/how-to-effectively-alias-a-user-on-linux>

Users can share their UID, giving them the same user permissions.

Some quick testing reveals that no matter which alias you are logged in as, the system will recognize you as the
first user which matches the UID from the passwd database (normally /etc/passwd but can be elsewhere in advanced
setups).

I have not seen that groups behave the same way, each alias needs to be added to each group explicitly. It would
seem that while users are mapped from UID, the groups look to be mapped from the name (check the contents of
/etc/group to get a sense of what I mean).

Quick example based on what you started with:

```sh

#!/bin/sh

USER1=primary
USER2=alias

sudo adduser $USER1
sudo useradd -d /home/$USER1 -G $USER1 -K UMASK=022 --non-unique --uid $(id -u $USER1) --no-create-home --no-user-group $USER2
sudo passwd $USER2

# --non-unique: Allow the creation of a user account with duplicate UID
# --uid: The numerical value of the user's ID
# --no-create-home: Do not create the user's home directory.
# --no-user-group: Do not create a group with the same name as the user.

```

Then how to set password non-interactively?

<https://stackoverflow.com/questions/65226720/setting-password-of-the-new-user-in-non-interactive-way-in-shell-script-on-ubunt>

```sh
echo "v3rystrongpassword" | passwd username --stdin
```

