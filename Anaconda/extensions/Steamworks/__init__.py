# Copyright (c) Mathias Kaerlev 2012.

# This file is part of Anaconda.

# Anaconda is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Anaconda is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

"""
Steam.mfx

Ported to Python by Mathias Kaerlev
"""

from mmfparser.player.extensions.common import UserExtension
from mmfparser.player.objects.common import ObjectPlayer
from mmfparser.player.event.actions.common import Action
from mmfparser.player.event.conditions.common import Condition, TrueCondition
from mmfparser.player.event.expressions.common import Expression
from wrapper import create_object

class DefaultObject(ObjectPlayer):
    def created(self):
        self.client = create_object()
        self.client.parent = self
        self.updateEnabled = True
    
    def update(self):
        self.client.update()
    
    def on_detach(self):
        pass
    
    def generate(self, id):
        condition = Steam.conditions[id]
        self.player.eventPlayer.generate_event(condition, self.parent.handle)

class ObjectAction(Action):
    def execute(self, instance):
        self.action(instance.objectPlayer.client)

class ObjectCondition(Condition):
    def check(self, instance):
        return self.condition(instance.objectPlayer.client)

class ObjectExpression(Expression):
    def get(self, instance):
        return self.expression(instance.objectPlayer.client)

# actions

class RequestData(ObjectAction):
    def action(self, client):
        client.request_data()

class StoreData(ObjectAction):
    def action(self, client):
        client.store_data()

class SetAchievement(ObjectAction):
    def action(self, client):
        name = self.evaluate_expression(self.get_parameter(0))
        client.set_achievement(name)

class SetIntStatistic(ObjectAction):
    def action(self, client):
        name = self.evaluate_expression(self.get_parameter(0))
        value = self.evaluate_expression(self.get_parameter(1))
        client.set_int_statistic(name, value)

class SetFloatStatistic(ObjectAction):
    def action(self, client):
        name = self.evaluate_expression(self.get_parameter(0))
        value = self.evaluate_expression(self.get_parameter(1))
        client.set_float_statistic(name, value)

class UpdateAvgrate(ObjectAction):
    def action(self, client):
        name = self.evaluate_expression(self.get_parameter(0))
        value = self.evaluate_expression(self.get_parameter(1))
        duration = self.evaluate_expression(self.get_parameter(2))
        client.update_avgrate(name, value, duration)

# conditions

class SteamInitialized(ObjectCondition):
    def condition(self, client):
        return client.is_initialized()

class DataReceived(TrueCondition):
    pass

class DataStored(TrueCondition):
    pass

class AchievementStored(TrueCondition):
    pass

# expressions

class GetAppID(ObjectExpression):
    def expression(self, client):
        return client.get_app_id()

class GetPlayerName(ObjectExpression):
    def expression(self, client):
        return client.get_player_name()

class GetIntStatistic(ObjectExpression):
    def expression(self, client):
        name = self.next_argument()
        return client.get_statistics_int(name)

class GetFloatStatistic(ObjectExpression):
    def expression(self, client):
        name = self.next_argument()
        return client.get_statistics_float(name)

class GetAchievementAPI(ObjectExpression):
    def expression(self, client):
        return client.get_achievement_api()

class GetCurrentProgress(ObjectExpression):
    def expression(self, client):
        return client.get_current_progress()

class GetMaximumProgress(ObjectExpression):
    def expression(self, client):
        return client.get_max_progress()

class GetAchievementDisplay(ObjectExpression):
    def expression(self, client):
        name = self.next_argument()
        return client.get_achievement_display(name)

class GetAchievementDescription(ObjectExpression):
    def expression(self, client):
        name = self.next_argument()
        return client.get_achievement_description(name)

class Steam(UserExtension):
    objectPlayer = DefaultObject
    
    actions = {
        1 : RequestData,
        5 : StoreData,
        0 : SetAchievement,
        2 : SetIntStatistic,
        3 : SetFloatStatistic,
        4 : UpdateAvgrate
    }
    
    conditions = {
        0 : SteamInitialized,
        1 : DataReceived,
        2 : DataStored,
        3 : AchievementStored
    }
    
    expressions = {
        0 : GetAppID,
        1 : GetPlayerName,
        7 : GetIntStatistic,
        8 : GetFloatStatistic,
        2 : GetAchievementAPI,
        3 : GetCurrentProgress,
        4 : GetMaximumProgress,
        5 : GetAchievementDisplay,
        6 : GetAchievementDescription
    }

extension = Steam()

def get_extension():
    return extension
