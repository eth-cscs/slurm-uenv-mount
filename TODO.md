# overview

## current

`mount`
- `is_valid_mount_point`
- `do_mount`

`util/helper`
- split
- is_sha
- is_file

`sqlite`:
The sqlite/sqlite.hpp header exposes an api around sqlite

`parse_args`:
- the `find_repo_image` function that returs the path of the image should be moved to the database

## proposed


`sqlite`:
- refactor so that it exposes an API around the database - we shouldn't leak underlying database implementation to the calling scope
- rename `db`

## sqlite


