# Lacewing.mfx

# Ported to Python by Mathias Kaerlev

from mmfparser.player.extensions.common import UserExtension
from mmfparser.player.objects.common import ObjectPlayer
from mmfparser.player.event.actions.common import Action
from mmfparser.player.event.conditions.common import Condition
from mmfparser.player.event.expressions.common import Expression
from wrapper import call_action, call_condition, call_expression, create_object

# Actions

class DefaultObject(ObjectPlayer):
    def created(self, data):
        self.client = create_object(self.parent, data)
        self.updateEnabled = True
    
    def update(self):
        self.client.update()
    
    def on_detach(self):
        self.client.on_detach()
    
    def generate(self, id):
        condition = Lacewing.conditions[id]
        self.player.eventPlayer.generate_event(condition, self.parent.handle)

def create_action(key):
    class ActionWrapper(Action):
        def execute(self, instance):
            call_action(instance, self, key)
    return ActionWrapper

def create_condition(key):
    class ConditionWrapper(Condition):
        def check(self, instance):
            return call_condition(instance, self, key)
    return ConditionWrapper

def create_expression(key):
    class ExpressionWrapper(Expression):
        def get(self, instance):
            return call_expression(instance, self, key)
    return ExpressionWrapper

class WrapperDict(object):
    def __init__(self, func):
        self.func = func
        self.cache = {}
    
    def __getitem__(self, key):
        try:
            return self.cache[key]
        except KeyError:
            pass
        item = self.func(key)
        self.cache[key] = item
        return item

class Lacewing(UserExtension):
    objectPlayer = DefaultObject
    
    actions = WrapperDict(create_action)
    
    conditions = WrapperDict(create_condition)
    
    expressions = WrapperDict(create_expression)

extension = Lacewing()

def get_extension():
    return extension
