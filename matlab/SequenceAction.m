% SequenceAction.m
% encoding: utf-8
%
% Class to represent a sequence of actions in a parametric sweep.
%
% Author:   Connor D. Pierce
% Created:  2023-04-06 13:36:39
% Modified: 2023-04-06 15:25:25
%
% Copyright (c) 2023 Connor D. Pierce
%
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
%
% The above copyright notice and this permission notice shall be included in all
% copies or substantial portions of the Software.
%
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
% SOFTWARE.
%
% SPDX-License-Identifier: MIT


% Perform a sequence of actions in a parametric sweep.
classdef SequenceAction < handle

   % Private properties
   properties (GetAccess = private, SetAccess = private)
      subactions(1, :) Action;
   end

   methods

      % Execute the action on the provided lock-in amplifier.
      %
      % :parameter obj: (Action)
      % :parameter lockin: (LockinAmplifier) interface to a lock-in amplifier
      function run(obj, lockin)
         arguments
            obj SequenceAction;
            lockin LockinAmplifier;
         end
         
         % Execute all the sub-actions one-by-one
         for action = obj.subactions
            run(action, lockin);
         end
      end

      % Add an action to the end of the sequence.
      %
      % :parameter obj: (SequenceAction)
      % :parameter action: (Action) action to append to the sequence
      function append(obj, action)
         arguments
            obj SequenceAction;
            action Action;
         end
         
         n = size(obj.subactions);
         obj.subactions(1, n + 1) = action;
      end

      % Insert an action at an arbitrary position in the sequence.
      %
      % :parameter obj: (SequenceAction)
      % :parameter action: (Action) action to insert in the sequence
      % :parameter pos: (int) position to insert the action
      function insert(obj, action, pos)
         arguments
            obj SequenceAction;
            action Action;
            pos int {mustBePositive};
         end

         n = size(obj.subactions, 2) + 1;
         newSubActions = zeros(1, n, Action);
         for i = 1:pos - 1
            newSubActions(i) = obj.subactions(i);
         end
         newSubActions(pos) = action;
         for i = pos + 1:n
            newSubActions(i) = obj.subactions(i - 1);
         end
         obj.subactions = newSubActions;
      end

      % Remove an action from the sequence.
      %
      % If the action occurs at multiple
      % :parameter obj: (SequenceAction)
      % :parameter action: (Action) action to remove from the sequence
      % :parameter which: (string) which occurrences to remove. Allowable values
      %    are 'first', 'last', and 'all'
      %
      % :return n: (int) the number of occurrences that were removed
      function n = remove(obj, action, which='first')
         arguments
            obj SequenceAction;
            action Action;
            pos int {mustBePositive};
            occurrence string {mustBeMember(which, {'first', 'last', 'all'})};
         end

         % Loop through all subactions and find the quantity and index of all
         % occurrences of `action` that match the specified `which`
         occurrences = zeros(1, size(obj.subactions), int);
         n = 0;
         if which == 'first'
            % Loop in forward order, break if an occurrence is found
            i = 1;
            for a = obj.subactions
               if a == action
                  n = n + 1;
                  occurrences(i) = 1;
                  break;
               end
               i = i + 1;
            end
         elseif  which == 'last'
            % Loop in reverse order, break if an occurrence is found
            i = size(obj.subactions, 2);
            for a = obj.subactions(1, ::-1)
               if a == action
                  n = n + 1;
                  occurrences(i) = 1;
                  break;
               end
               i = i - 1;
            end
         elseif which == 'all'
            % Loop through all subactions without breaking
            i = 1;
            for a = obj.subactions
               if a == action
                  n = n + 1;
                  occurrences(i) = 1;
               end
               i = i + 1;
            end
         end

         % Leave the function if no occurrences were found
         if n == 0
            return;
         end

         % Create new subactions list of the appropriate size
         new_n = size(obj.subactions, 2) - n;
         newSubActions = zeros(1, new_n, Action);

         % Counter to keep track of how many items have been omitted in building the
         % new list
         skipped = 0;

         % Loop through the current items and add them to the new list unless they are
         % identified as occurrences of `action`
         for i = 1:size(obj.subactions, 2)
            if occurrences(i)
               % This entry should be skipped
               %   1. Increment `skipped`
               %   2. Don't copy this action to the new list
               skipped = skipped + 1;
            else
               % This entry should be retained
               %   1. Don't increment skipped
               %   2. Copy this item to the appropriate place in the new list, which is
               %      its former location minus the number of items skipped so far
               newSubActions(i - skipped) = obj.subactions(i);
            end
         end

         % Assign the new list of subactions to this object
         obj.subactions = newSubActions;
      end

   end

end
