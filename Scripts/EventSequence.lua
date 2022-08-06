local Timer = require("Timer")

local EventSequence

LevelVars.__TEN_eventSequence = {sequences = {}}

LevelFuncs.__TEN_eventSequence_callNext = function(sequenceName, nextTimerName, func, ...)
	local thisES = LevelVars.__TEN_eventSequence.sequences[sequenceName]
	LevelFuncs[func](...)

	thisES.currentTimer = thisES.currentTimer + 1
	if thisES.currentTimer <= #thisES.timers then
		local theTimer = Timer.Get(nextTimerName)
		theTimer:SetRemainingTime(theTimer:GetTotalTime())
		theTimer:Start()
	elseif thisES.loop then
		local theTimer = Timer.Get(thisES.firstTimerName)
		theTimer:SetRemainingTime(theTimer:GetTotalTime())
		theTimer:Start()
		thisES.currentTimer = 1
	end
end

EventSequence = {
	Create = function(name, loop, showString, ...)
		local obj = {}
		local mt = {}
		mt.__index = EventSequence
		setmetatable(obj, mt)

		obj.name = name

		LevelVars.__TEN_eventSequence.sequences[name] = {} 
		local thisES = LevelVars.__TEN_eventSequence.sequences[name]
		thisES.name = name
		thisES.timesFuncsAndArgs = {...}
		thisES.loop = loop
		
		local tfa = thisES.timesFuncsAndArgs
		thisES.timers = {}
		thisES.currentTimer = 1
		local prevTimer = nil
		local prevFuncName = nil

		for i = 1, #tfa, 2 do
			local nextTimer = i + 2
			local timerIndex = #thisES.timers + 1 

			local funcName = "__TEN_eventSequence_" .. name .. "_func" .. timerIndex 
			local timerName = "__TEN_eventSequence_" .. name .. "_timer" .. timerIndex 
			local nextTimerName = "__TEN_eventSequence_" .. name .. "_timer" .. timerIndex + 1

			local funcAndArgs = tfa[i+1]
			local func

			if i == 1 then
				thisES.firstTimerName = timerName
			end

			if type(funcAndArgs) == "string" then
				-- we only have a function
				func = funcAndArgs
				funcAndArgs = {}
			else
				-- we have a function and possible args
				func = table.remove(funcAndArgs, 1)
			end

			local thisTimer = Timer.Create(timerName,
						tfa[i], -- time
						false,
						showString,
						"__TEN_eventSequence_callNext",
						name,
						nextTimerName,
						func,
						table.unpack(funcAndArgs) -- now with func removed
						)

			thisES.timers[timerIndex] = timerName
		end

		return obj
	end;

	Get = function(name)
		if LevelVars.__TEN_eventSequence.sequences[name] then
			local obj = {}
			local mt = {}
			mt.__index = EventSequence
			setmetatable(obj, mt)
			obj.name = name
			return obj
		end
		return nil
	end,

	Pause = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Pause()
	end,

	Stop = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Stop()
	end,

	Start = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Start()
	end;
}

return EventSequence
