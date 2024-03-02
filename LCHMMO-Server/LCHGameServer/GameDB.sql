CREATE PROCEDURE spJoinPlayer
(
@id nvarchar(30),
@pwd nvarchar(30)
)
as
begin
	set nocount on
	declare @result int
	set @result = 0

	declare @rowcnt int

	select @rowcnt = count(*) from Player 
	where id = @id and pwd = @pwd

	if @rowcnt = 0
	begin
		insert into Player(id, pwd) values(@id, @pwd)
	end

	else
	begin
		set @result = 1
	end

	select @result as result
end

CREATE PROCEDURE spLoginPlayer
(
@id nvarchar(30),
@pwd nvarchar(30)
)
as
begin
	set nocount on
	declare @result int
	set @result = 0

	declare @rowcnt int

	select @rowcnt = count(*) from Player
	where id = @id and pwd = @pwd

	if @rowcnt <> 1
	begin
		set @result = 1
	end

	select @result as result
end