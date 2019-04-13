# CTF Walkthrough
This code should not be released with the files private.key, flag.txt or this walkthrough. Remove before making public. 

Steps:
* Try to read flag.txt from browser, not possible.
* Checking source code tells you implementation is impossible. 
* Modifying code and try to upload using browser.
* Signing failed.
* Theres an image named X.key.jpg
* Using stego
```
steghide extract -sf image.jpg 
```
* Password on back of badge (n00badge)
* Get private.key file
* Decode base64 private key.
* Signes new firmware with private key.
* Upload signed and modified binary using browser.
* Get flag
