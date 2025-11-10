

CREATE TABLE Servers(server_id int unique not null,server_name Varchar(256) unique not null, chunk_size int, server_ip Varchar(256), primary key(server_id));
