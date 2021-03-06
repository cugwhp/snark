/**
 * Created by vrushali on 15/10/15.
 */
define('GraphFeed', ["jquery", "Feed"], function ($) {
    var Feed = require('Feed');

    var GraphFeed = function (feed_name, feed_path, config) {
        this.base = Feed;
        this.base(feed_name, feed_path, config);
        this.default_threshold = {value: this.config.graph.max, color: '#5cb85c'};
        this.default_exceeded_threshold = {value: Number.MAX_VALUE, color: '#d9534f', alert: true};
        this.text = $(this.id + ' .graph-text');
        this.y_labels = $(this.id + ' .graph-y-labels');
        this.bars = $(this.id + ' .graph-bars');
        this.set_labels();
    };
    GraphFeed.prototype = Object.create(Feed.prototype);
    GraphFeed.prototype.set_labels = function () {
        this.y_labels.empty();
        this.bars.empty();
        this.bars.append('<div class="graph-bar-col"><span class="graph-bar-bottom"></span><span class="graph-bar-top"></span></div>');
        this.bars_width = Number($('.graph-bars').css('width').replace('px', ''));
        this.bar_width = Number($('.graph-bar-col').css('width').replace('px', ''));
        this.bar_height = Number($('.graph-bar-col').css('height').replace('px', ''));
        this.bar_count = Math.floor(this.bars_width / this.bar_width);
        if (!isFinite(this.bar_count)) {
            throw 'invalid graph bar styles; calculated bar count: ' + this.bar_count;
        }
        for (var i = 1; i < this.bar_count; ++i) {
            this.bars.append('<div class="graph-bar-col"><span class="graph-bar-bottom"></span><span class="graph-bar-top"></span></div>');
        }
        var _this = this;
        var got_max = false;
        this.config.graph.thresholds.forEach(function (threshold, index) {
            if (threshold.value < _this.config.graph.min || threshold.value > _this.config.graph.max) {
                return;
            }
            if (threshold.value == _this.config.graph.max) {
                got_max = true;
            }
            var span = $('<span class="graph-y-label">' + threshold.value + _this.config.graph.units + '</span>');
            span.css('top', _this.get_bar_height(threshold.value) - 9);
            span.appendTo(_this.y_labels);
        });
        if (!got_max) {
            var span = $('<span class="graph-y-label">' + this.config.graph.max + _this.config.graph.units + '</span>');
            span.css('top', _this.get_bar_height(this.config.graph.max) - 9);
            span.appendTo(this.y_labels);
        }
        {
            var span = $('<span class="graph-y-label">' + this.config.graph.min + _this.config.graph.units + '</span>');
            span.css('top', _this.get_bar_height(this.config.graph.min) - 9);
            span.appendTo(this.y_labels);
        }
        this.bars.find('.graph-bar-col').each(function (i, e) {
            _this.config.graph.thresholds.forEach(function (threshold, index) {
                if (threshold.value > _this.config.graph.max) {
                    return;
                }
                var span = $('<span class="graph-threshold"></span>');
                span.css('height', _this.get_bar_height(threshold.value) + 1 + 'px');
                span.css('border-bottom-color', threshold.color);
                span.appendTo(e);
            });
        });
        $('.graph-bar-col').tooltip();
    }
    GraphFeed.prototype.load = function () {
        $.ajax({
            context: this,
            crossDomain: true,
            url: this.get_url(),
            timeout: globals.timeout
        }).done(function (data, textStatus, jqXHR) {
            this.onload(data);
        }).fail(function (jqXHR, textStatus, errorThrown) {
            this.onerror();
        });
    };
    GraphFeed.prototype.onload_ = function (data) {
        var text = data ? data.replace("\n", '') : 'N/A';
        if (this.config.graph.units) {
            text += ' ' + this.config.graph.units;
        }
        this.text.html(text);
        var value = this.config.graph.min < 0 && this.config.graph.max > 0 ? 0 : this.config.graph.max <= 0 ? this.config.graph.max : this.config.graph.min;
        if (!isNaN(data)) {
            value = Number(data);
        }
        var bar = this.bars.children().first();
        this.bars.append(bar);
        var bottom_height = this.get_bar_bottom(value);
        var top_height = this.get_bar_top(value);
        var threshold = this.get_threshold(value);
        bar.find('.graph-bar-bottom').css('height', bottom_height).css('background', threshold.color);
        bar.find('.graph-bar-top').css('height', top_height);
        if (this.config.alert) {
            this.alert(threshold.alert);
        }
        bar.data('bs.tooltip').options.title = text + ' @ ' + this.refresh_time;
        bar.find('.graph-threshold').each(function (index, value) {
            var e = $(value);
            var height = Number(e.css('height').replace('px', ''));
            if (height > top_height && height < bottom_height) {
                e.hide();
            } else {
                e.show();
            }
        });
    };
    GraphFeed.prototype.get_bar_height = function (value) {
        var scale = (value - this.config.graph.min) / (this.config.graph.max - this.config.graph.min);
        if (scale > 1) {
            scale = 1;
        } else if (scale < 0) {
            scale = 0;
        }
        return this.bar_height - scale * this.bar_height;
    };
    GraphFeed.prototype.get_bar_top = function (value) {
        if (this.config.graph.max > 0 && this.config.graph.min < 0) {
            if (value < 0) {
                return this.get_bar_height(0);
            }
            return this.get_bar_height(value);
        }
        if (this.config.graph.max <= 0) {
            return this.get_bar_height(this.config.graph.max);
        }
        return this.get_bar_height(value);
    };
    GraphFeed.prototype.get_bar_bottom = function (value) {
        if (this.config.graph.max > 0 && this.config.graph.min < 0) {
            if (value >= 0) {
                return this.get_bar_height(0);
            }
            return this.get_bar_height(value);
        }
        if (this.config.graph.min >= 0) {
            return this.get_bar_height(this.config.graph.min);
        }
        return this.get_bar_height(value);
    };
    GraphFeed.prototype.get_threshold = function (value) {
        if (!this.config.graph.thresholds.length) {
            return this.default_threshold;
        }
        for (var i in this.config.graph.thresholds) {
            var threshold = this.config.graph.thresholds[i];
            if (value <= threshold.value) {
                return threshold
            }
        }
        return this.default_exceeded_threshold;
    };
    return GraphFeed;
});
