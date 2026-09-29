# AWS Deployment Setup Guide

This guide explains how to properly configure GitHub Secrets for the automated AWS deployment.

## Prerequisites

Before setting up deployment, you need:
- An AWS EC2 instance running and accessible via SSH
- An SSH key pair (.pem file) for your EC2 instance
- Access to your GitHub repository settings

## Step-by-Step Setup

### Step 1: Obtain Your SSH Private Key (.pem file)

1. **Find your .pem file** on your local machine
   - If you created the EC2 instance via AWS Console, you downloaded it when creating the key pair
   - Look in your Downloads folder or wherever you saved it
   - The file name typically ends with `.pem` (e.g., `my-ec2-key.pem`)

2. **Verify it's the correct key**
   - The file should contain:
     - Start: `-----BEGIN RSA PRIVATE KEY-----` (or similar)
     - End: `-----END RSA PRIVATE KEY-----`
     - Approximately 1500-3000 lines of base64-encoded text

### Step 2: Prepare the Private Key Content

**IMPORTANT:** Copy the ENTIRE file content, including the BEGIN and END lines.

#### On Windows:
1. Right-click the `.pem` file → Open with → Notepad (or VS Code)
2. Press `Ctrl+A` to select all text
3. Press `Ctrl+C` to copy
4. **Paste into a temporary text editor** to verify it includes:
   - Line 1: `-----BEGIN RSA PRIVATE KEY-----`
   - Last line: `-----END RSA PRIVATE KEY-----`
   - All lines in between (should be substantial!)

#### On Mac/Linux:
```bash
cat /path/to/your/key.pem
```
Copy the entire output from the terminal.

### Step 3: Get Your EC2 Instance Information

1. Go to **AWS Console** → **EC2** → **Instances**
2. Select your instance
3. Note down:
   - **Public IPv4 address** or **Public DNS** - This is your `AWS_INSTANCE_HOST`
     - Example: `ec2-1-2-3-4.compute.amazonaws.com` or `3.14.159.26`
   - **SSH username** - Depends on AMI:
     - Ubuntu: `ubuntu`
     - Amazon Linux: `ec2-user`
     - This is your `AWS_SSH_USER`

### Step 4: Add Secrets to GitHub Repository

1. Go to your GitHub repository
2. Click **Settings** (top menu)
3. Click **Secrets and variables** → **Actions** (left sidebar)
4. Click **New repository secret** (green button)

#### Secret 1: AWS_INSTANCE_HOST

- **Name:** `AWS_INSTANCE_HOST`
- **Value:** Paste your EC2 public IP or DNS
  - Example: `ec2-1-2-3-4.compute.amazonaws.com`
- Click **Add secret**

#### Secret 2: AWS_SSH_USER

- **Name:** `AWS_SSH_USER`
- **Value:** Your SSH username
  - Example: `ubuntu` or `ec2-user`
- Click **Add secret**

#### Secret 3: AWS_SSH_PRIVATE_KEY

- **Name:** `AWS_SSH_PRIVATE_KEY`
- **Value:** The complete contents of your `.pem` file
  - **PASTE THE ENTIRE FILE** including:
    ```
    -----BEGIN RSA PRIVATE KEY-----
    MIIEpAIBAAKCAQEA...extremely long line...
    ...many more lines...
    ...very last line...
    -----END RSA PRIVATE KEY-----
    ```
  - **DO NOT add extra text, spaces, or blank lines**
  - **DO NOT split it across multiple lines in the UI** - paste it as-is
- Click **Add secret**

### Step 5: Verify Secret Upload

After adding all three secrets:

1. Go back to **Settings** → **Secrets and variables** → **Actions**
2. You should see all three secrets listed:
   - ✅ AWS_INSTANCE_HOST
   - ✅ AWS_SSH_USER
   - ✅ AWS_SSH_PRIVATE_KEY

**Note:** GitHub will not show the secret values for security. They will appear as masked dots.

### Step 6: Test the Deployment

1. Go to **Actions** tab in your GitHub repository
2. Click **CI - Build, Test, and Lint** workflow
3. Click **Run workflow** → **Run workflow**

The workflow will:
- Build and test your code
- Validate the secrets
- Deploy to your AWS instance
- Create a deployment status file

You should see output showing:
- ✅ SSH key setup complete
- ✅ AWS Instance Information
- ✅ Deployment Status File

### Troubleshooting

#### Issue: "SSH key file missing 'BEGIN PRIVATE KEY' marker"

**Cause:** The key content is incomplete or corrupted.

**Fix:**
1. Delete the current `AWS_SSH_PRIVATE_KEY` secret
2. Re-open your `.pem` file in a text editor
3. Verify it starts with `-----BEGIN RSA PRIVATE KEY-----`
4. Verify it ends with `-----END RSA PRIVATE KEY-----`
5. Copy the **exact content** from the file
6. Paste into GitHub secret (ensure no accidental modifications)
7. Re-run the workflow

#### Issue: "SSH connection failed"

**Possible causes:**
1. Wrong `AWS_INSTANCE_HOST` - verify EC2 public IP/DNS
2. Wrong `AWS_SSH_USER` - verify the AMI type matches
3. Security group blocking SSH - ensure port 22 is open in EC2 security group
4. EC2 instance is not running - verify instance is in "running" state

**Fix:**
1. Test SSH locally first:
   ```bash
   ssh -i /path/to/key.pem ubuntu@your-ec2-public-ip
   ```
2. If local SSH works, verify the GitHub secrets are correct
3. Re-run the workflow

#### Issue: "Deploy file not created successfully"

**Cause:** SSH connection was successful but file creation failed.

**Possible reasons:**
- Home directory permissions issue
- Insufficient disk space on EC2
- User doesn't have write permissions

**Fix:**
1. SSH to EC2 and check disk space:
   ```bash
   ssh -i /path/to/key.pem ubuntu@your-ec2-public-ip
   df -h
   ```
2. Check home directory permissions:
   ```bash
   ls -la ~
   ```
3. Try creating a test file manually:
   ```bash
   touch ~/test-file.txt
   ```

## Security Considerations

- **Never commit your .pem file** to the repository
- **Never paste your .pem file** in chat or documentation
- **Use GitHub Secrets** for all sensitive credentials
- **Rotate your EC2 key pair** periodically
- **Restrict SSH key file permissions** to 600 (`chmod 600 key.pem`)

## Additional Information

The deployment workflow creates a status file at:
```
~/dcm-mvp-deployment-status.txt
```

This file contains:
- Deployment timestamp (in IST format)
- Git branch name
- Commit SHA
- Commit message
- Pipeline status

To verify deployments from your AWS instance:
```bash
ssh -i /path/to/key.pem ubuntu@your-ec2-public-ip
cat ~/dcm-mvp-deployment-status.txt
```

---

**Need help?** Check the GitHub Actions logs for detailed error messages:
1. Go to **Actions** tab
2. Click the failed run
3. Click **deploy** job
4. Review the step-by-step output

