@ECHO OFF

REM A batch script to generate SHA1 files to upload
REM sha1sum must be in your PATH to use this
REM You can snag it from here ftp://ftp.gnupg.org/gcrypt/binary/sha1sum.exe

SETLOCAL ENABLEDELAYEDEXPANSION
IF EXIST bin\NUL (
    CHDIR bin
    FOR /F "tokens=*" %%F IN ('dir /b *.exe') DO (
        IF /I NOT %%F == "File Not Found" (
            IF NOT EXIST "%%F.SHA1" (
                sha1sum %%F > %%F.SHA1
                IF EXIST "%%F.SHA1" (
                    ECHO %%F.SHA1 created
                )
            ) ELSE (
                FOR /F "usebackq delims=" %%G IN ("%%F.SHA1") DO (
                    FOR /F "tokens=*" %%H IN ('sha1sum %%F') DO (
                        IF NOT "%%H" == "" (
                            IF /I "%%H" == "%%G" (
                                ECHO %%F.SHA1 already exists
                            ) ELSE (
                                sha1sum %%F > %%F.SHA1
                                IF EXIST "%%F.SHA1" (
                                ECHO %%F.SHA1 has been updated
                                )
                            )
                        )
                    )
                )
            )
        )
    )
    CHDIR ..
) ELSE (
    ECHO bin dir not found!
    pause
)