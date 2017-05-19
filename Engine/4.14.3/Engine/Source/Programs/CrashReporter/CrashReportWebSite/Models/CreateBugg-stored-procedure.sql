﻿USE [CrashReport]
GO

/****** Object:  StoredProcedure [dbo].[UpdateCrashesByPattern]    Script Date: 2015-12-21 12:19:27 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[UpdateCrashesByPattern]
AS


BEGIN
	DECLARE
	@CurrRunTS datetime,
	@LastRunTS datetime

    SET @CurrRunTS = GETDATE()
	SET @LastRunTS = 
	    (Select LastRunTime
		 from [dbo].[CrashesHWM]
		 WHERE ID = 1
	    )
	
	
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

	
	-- Update crash user names

	UPDATE [dbo].[Crashes]
	SET [UserName] = u.[UserName]
    FROM [dbo].[Crashes] c
	LEFT JOIN [dbo].[Users] u on (u.[id] = c.[UserNameId])
	WHERE c.UserName IS NULL;

	
	--Create Buggs
	-- #1 most expensive query (~50%?)
	-- Changed on 2016-01-26 to only read new crashes, by looking at the insert date of the crash records. 
	-- Merge statement changed to increment NumberOfCrashes and NumberOfUsers rather than calculate the entire thing.

	MERGE Buggs AS B
	USING 

	(
		SELECT 
		
			   [TTPID]
			  ,[Pattern]
			  ,[NumberOfCrashes]
			  ,[NumberOfUsers]
			  ,[TimeOfFirstCrash]
			  ,[TimeOfLastCrash]
			  ,[BuildVersion]
			  ,[CrashType]
			  ,[Status]
			  ,[FixedChangeList]
			  
		FROM

		(
			SELECT COUNT(1) as NumberOfCrashes
				  , max([TimeOfCrash]) as TimeOfLastCrash
				  , min([TimeOfCrash]) as TimeOfFirstCrash
				  , max([BuildVersion]) as BuildVersion
				  , max([CrashType]) as CrashType
				  , count(distinct [UserName]) as NumberOfUsers
				  , Max([Status]) as [Status]
				  , Max(TTPID) as TTPID
				  , Max(FixedChangeList) as FixedChangeList
				  , Pattern
			  FROM [dbo].[Crashes] c 
			  WHERE
				  Pattern is not NULL 
			  AND Pattern not like ''
			  AND InsertTS between @LastRunTS and @CurrRunTS
			  group by Pattern
		  ) as s
	) AS C 
	ON (B.Pattern = C.Pattern)

	WHEN NOT MATCHED BY TARGET
		THEN INSERT (
		       [TTPID]
			  ,[Pattern]
			  ,[NumberOfCrashes]
			  ,[NumberOfUsers]
              ,[TimeOfFirstCrash]
			  ,[TimeOfLastCrash]
              ,[BuildVersion]
			  ,[CrashType]
			  ,[Status]
		  	  ,[FixedChangeList]

			  ) 
		Values(
        		C.[TTPID]
			  , C.[Pattern]
			  , C.[NumberOfCrashes]
			  , C.[NumberOfUsers]
              , C.[TimeOfFirstCrash]
			  , C.[TimeOfLastCrash]
              , C.[BuildVersion]
			  , C.[CrashType]
			  , C.[Status]
			  , C.[FixedChangeList]
			  ) 

	WHEN MATCHED 
		THEN UPDATE SET   B.[NumberOfCrashes] = B.[NumberOfCrashes] + C.[NumberOfCrashes]
         				, B.[TimeOfLastCrash] = C.[TimeOfLastCrash]
						, B.[BuildVersion] = C.[BuildVersion]
                        , B.[CrashType] = C.[CrashType]
                        , B.[NumberOfUsers] = B.[NumberOfUsers] + C.[NumberOfUsers]

	OUTPUT $action, Inserted.*, Deleted.*;
		

	/****** Join Buggs and Crashes  ******/
	MERGE dbo.Buggs_Crashes BC 

	USING 
	( 

		SELECT  b.Id as BuggId, 
		        c.[Id] as CrashId
		FROM [dbo].[Crashes] c
        Join [dbo].Buggs b on (b.Pattern = c.Pattern)
		WHERE C.[InsertTS] between @LastRunTS and @CurrRunTS
		group by b.Id, c.Id

	 ) AS C
	 ON BC.BuggId = C.BuggId AND BC.CrashId = C.CrashId

	 WHEN NOT MATCHED BY TARGET
		THEN INSERT 
        	(BuggId, CrashId)
		VALUES 

			(C.BuggId, C.CrashId)	
	OUTPUT $action, Inserted.*, Deleted.*;


	/****** Join Buggs_Users and Crashes  ******/
	MERGE dbo.Buggs_Users AS BU

	USING
		(

		  SELECT  b.Id as BuggId, c.[UserName] as UserName
		  FROM [dbo].[Crashes] c
		  Join [dbo].Buggs b on (b.Pattern = c.Pattern)
		  WHERE c.UserName IS NOT NULL
		  AND c.InsertTS between @LastRunTS and @CurrRunTS
		  Group by b.Id, c.UserName

		) AS C
	ON BU.BuggId = C.BuggId AND BU.UserName = C.UserName

	 WHEN NOT MATCHED BY TARGET
		THEN INSERT 
        	(BuggId, UserName) 
		VALUES 
        	(C.BuggId, C.UserName)	
	 WHEN MATCHED
    	THEN UPDATE SET
			BU.BuggId = C.BuggId, 
            BU.UserName = C.UserName
	OUTPUT $action, Inserted.*, Deleted.*;


	/****** Join Buggs and UserGroups  ******/
	MERGE dbo.Buggs_UserGroups AS BUG
	USING
	(

	  Select BuggId, UserGroupId
	  From [dbo].[Buggs_Users] bu
	  Join dbo.Users u on (u.UserName = bu.UserName)
	  Group by BuggId, UserGroupId
	 ) AS BU
	 ON  BUG.BuggId = BU.BuggId AND BUG.UserGroupId = BU.UserGroupId

	 WHEN NOT MATCHED BY TARGET
		THEN INSERT 
        	(BuggId, UserGroupId) 
		VALUES 
        	(BU.BuggId, BU.UserGroupId)	
	 WHEN MATCHED
    	THEN UPDATE SET
			BUG.BuggId = BU.BuggId, 
            BUG.UserGroupId = BU.UserGroupId
	OUTPUT $action, Inserted.*, Deleted.*;

	 
	/*** Update Crashes to match the Bugg Status, TTPID, and Fixed Change List ****/

	-- This handles new crashes that enter the system; it has the side effect of making the Bugg authoritative in this case but that's how they should be used.
	-- #2 most expensive query (~20%?)
	update C
	SET 
		C.TTPID = B.TTPID,
        C.FixedChangeList = B.FixedChangeList,
		C.[Status] = B.[Status]
	From 
		dbo.Crashes as C
    Join Buggs_Crashes as BC ON (c.Id = BC.CrashId)
	Join Buggs as B ON (B.Id =BC.BuggId AND C.Id = BC.CrashId) 
	where C.TimeOfCrash > DATEADD(DD, -7, GETDATE())
	-- The time criteria here was added to the proc on 2016-01-26 to limit the crashes that are being updated to having happened in the last week
	-- This could be wrong, and we may have to pull this date criterion out if things don't update correctly, 
	-- but I wanted to limit the amount of data that we were working with as much as possible. 


--set our high watermark
	 UPDATE dbo.CrashesHWM 
	 SET LastRunTime = @CurrRunTS
	 WHERE ID = 1

END
GO

