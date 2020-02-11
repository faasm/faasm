# Faasm Releases

## Commands

Note: you must make sure you've _already_ updated the versions where necessary.

```bash
inv gh_create_release
inv gh_upload_artifacts
```

Now check the draft release. If it's ok:

```bash
inv gh_push_release
```

## Github config

You'll need to create a public access token with the relevant permissions and 
add it to `~/faasm/faasm.ini`:

```ini
[Github]
access_token: <token>
``` 