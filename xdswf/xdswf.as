package {

import com.qorporation.Connection;
import com.qorporation.Request;
import com.qorporation.Upload;

import flash.display.Sprite;
import flash.display.Stage;
import flash.display.StageAlign;
import flash.display.StageScaleMode;
import flash.external.ExternalInterface;
import flash.events.*;
import flash.system.Security;

import com.adobe.serialization.json.JSON;

public class xdswf extends Sprite {
    
    private var m_commChannel:Connection = null;
    private var m_uploadId:uint = 0;
    private var m_uploads:Object = {};
    
    public function xdswf() {
        trace("360io: initializing xdswf");
        
        Security.allowDomain("*");

        this.stage.align = StageAlign.TOP_LEFT;
        this.stage.scaleMode = StageScaleMode.NO_SCALE;
        
        var _sprite:Sprite = new Sprite();
        _sprite.x = 0;
        _sprite.y = 0;
        _sprite.buttonMode = true;
        _sprite.mouseChildren = false;
        _sprite.addEventListener(MouseEvent.CLICK, function (event:MouseEvent):void {
            extSelectUpload(m_uploadId);
        });

        this.stage.addChild(_sprite);
        this.stage.addEventListener(Event.RESIZE, function(event:Event):void {
            _sprite.graphics.clear();
            _sprite.graphics.beginFill(0x000000, 0);
            _sprite.graphics.drawRect(0, 0, event.target.stageWidth, event.target.stageHeight);
            _sprite.graphics.endFill();
        });

        trace("360io: registering hooks");
        ExternalInterface.addCallback('request', extRequest);
        ExternalInterface.addCallback('initComm', extInitComm);
        ExternalInterface.addCallback('initUpload', extInitUpload);
        ExternalInterface.addCallback('selectUpload', extSelectUpload);
        ExternalInterface.addCallback('cancelUpload', extCancelUpload);
        ExternalInterface.addCallback('runUpload', extRunUpload);
        
        trace("360io: calling ready");
        ExternalInterface.call('io360.xdswf.ready');
        
        trace("360io: xdwf loaded");
    };
    
    public function extRequest(path:String, args:Object, callback:String):void {
        var request:Request = new Request(path, args, function(res:String):void {
            if (res.charAt(0) != '{') res = "'" + res + "'";
            ExternalInterface.call("function() { " + callback + "(" + res + "); }");
        });
        
        request.run();
    };
    
    public function extInitComm(path:String, callback:String):void {
        if (m_commChannel) {
            m_commChannel.destroy();
        }
        
        m_commChannel = new Connection(path, function(res:String):void {
            if (res.charAt(0) != '{') res = "'" + res + "'";
            ExternalInterface.call("function() { " + callback + "(" + res + "); }");
        });
    };
    
    public function extInitUpload(types:String, onSelect:String, onProgress:String, onComplete:String):void {
        m_uploads[++m_uploadId] = new Upload(m_uploadId, types, function(id:Number, name:String, size:Number):void {
            ExternalInterface.call(onSelect, id, name, size);
        }, function(id:Number, loaded:Number, total:Number):void {
            ExternalInterface.call(onProgress, id, loaded, total);
        }, function(id:Number, res:String):void {
            if (res.charAt(0) != '{') res = "'" + res + "'";
            ExternalInterface.call("function() { " + onComplete + "(" + id + "," + res + "); }");
        });  
    };
    
    public function extSelectUpload(id:Number):void {
        m_uploads[id].select();
    };
    
    public function extCancelUpload(id:Number):void {
        m_uploads[id].cancel();
    };
    
    public function extRunUpload(id:Number, path:String, args:Object):void {
        m_uploads[id].run(path, args);
    };
    
};

}